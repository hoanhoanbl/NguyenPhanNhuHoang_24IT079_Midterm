# Đồ án giữa kỳ – Cài đặt lệnh `ls(1)`

## Thông tin sinh viên

| | |
|---|---|
| Họ và tên | Nguyễn Phan Như Hoàng |
| Mã số sinh viên | 24IT079 |
| Môn học | Lập trình hệ thống |
| Repository GitHub | https://github.com/hoanhoanbl/NguyenPhanNhuHoang_24IT079_Midterm |

## 1. Giới thiệu

Đồ án này cài đặt từ đầu, bằng ngôn ngữ C, một phiên bản đơn giản hóa của lệnh
`ls(1)` trên UNIX. Hành vi của chương trình bám theo trang manual `ls(1)` của
NetBSD (phiên bản 10.1) được giao kèm đề bài, với các tùy chọn:

```
ls [-AacdFfhiklnqRrSstuw] [file ...]
```

Chương trình chỉ dùng các lời gọi hệ thống và thư viện C (`stat`, `lstat`,
`opendir`, `readdir`, `readlink`, `getpwuid`, `getgrgid`, `getopt`, ...). Chương
trình không gọi lệnh `ls` thật và không dùng `strmode(3)` hay
`humanize_number(3)`: chuỗi quyền (mode) và kích thước dạng dễ đọc đều do dự án
tự cài đặt.

**Môi trường:** NetBSD 10.1 (amd64) chạy trên VirtualBox, trình biên dịch `cc`
(GCC 10.5.0), `make`.

## 2. Cấu trúc dự án

Mã nguồn được chia thành các module, mỗi module gồm một tập tin `.c` và một
tập tin header `.h`:

| Module | Nhiệm vụ |
|---|---|
| `main.c` | Điểm vào chương trình: phân tích tùy chọn, liệt kê các đối số, trả về mã thoát |
| `options.c/.h` | Phân tích dòng lệnh bằng `getopt(3)`, cấu trúc `struct options`, quy tắc cho các tùy chọn ghi đè lẫn nhau |
| `entry.c/.h` | `struct entry` (tên, đường dẫn, `struct stat`), danh sách mục có thể mở rộng, chọn mốc thời gian (`-c`, `-u`) |
| `list.c/.h` | Xử lý các đối số (operand), đọc thư mục, `-a`, `-A`, `-d`, `-R` |
| `sort.c/.h` | Sắp xếp các mục: theo tên, `-t`, `-S`, `-r`, `-f` |
| `format.c/.h` | Chuyển thuộc tính tập tin thành văn bản: chuỗi quyền, ngày giờ, kích thước (`-h`, `-k`, `BLOCKSIZE`), ký tự `-F` |
| `print.c/.h` | In kết quả: dạng ngắn, `-i`, `-s`, dạng dài `-l`/`-n`, căn cột, dòng `total` |
| `util.c/.h` | Cấp phát bộ nhớ có kiểm tra lỗi, báo lỗi |
| `Makefile` | Quy tắc biên dịch (`make`, `make clean`, `make test`) |
| `test.sh` | Tự động so sánh kết quả với lệnh `ls` của hệ thống |

## 3. Biên dịch và chạy

```sh
make            # biên dịch, tạo tập tin thực thi ./myls
make clean      # xóa các tập tin .o và tập tin thực thi
make test       # chạy test.sh (so sánh ./myls với /bin/ls)
```

Cách dùng:

```sh
./myls                  # liệt kê thư mục hiện tại
./myls -l /etc          # định dạng dài
./myls -laR ~           # tất cả mục, định dạng dài, đệ quy
./myls -lhS file1 dir1  # kích thước dễ đọc, lớn nhất trước
```

Tập tin thực thi có tên `myls` để không nhầm với lệnh `ls` của hệ thống. Thông
báo lỗi dùng tên chương trình (ví dụ `myls: file: No such file or directory`).

## 4. Các tính năng đã cài đặt

Tất cả các tùy chọn trong trang manual đều được cài đặt.

| Tùy chọn | Hành vi |
|---|---|
| `-A` | Liệt kê mọi mục trừ `.` và `..`; luôn bật với siêu người dùng (root) |
| `-a` | Bao gồm cả các mục có tên bắt đầu bằng dấu chấm |
| `-c` | Dùng thời điểm đổi trạng thái tập tin để sắp xếp (`-t`) và in ra (`-l`) |
| `-d` | Liệt kê thư mục như tập tin thường, không đi theo liên kết mềm trong danh sách đối số |
| `-F` | Thêm sau tên: `/` (thư mục), `*` (thực thi), `@` (liên kết mềm), `%` (whiteout), `=` (socket), `\|` (FIFO) |
| `-f` | Không sắp xếp đầu ra |
| `-h` | Kích thước dạng dễ đọc (`512B`, `1.5K`, `23M`) với `-s` và `-l`; ghi đè `-k` |
| `-i` | In số hiệu inode |
| `-k` | Số khối tính theo kilobyte với `-s`; trong `-k` và `-h`, cờ ngoài cùng bên phải thắng |
| `-l` | Định dạng dài (xem bên dưới) |
| `-n` | Giống `-l` nhưng chủ sở hữu và nhóm hiển thị dạng số |
| `-q` | Ký tự không in được trong tên hiển thị thành `?` (mặc định khi ra terminal) |
| `-R` | Liệt kê đệ quy các thư mục con (không đi theo liên kết mềm) |
| `-r` | Đảo ngược thứ tự sắp xếp |
| `-S` | Sắp xếp theo kích thước, lớn nhất trước |
| `-s` | In số khối đĩa mà mỗi tập tin sử dụng (đơn vị 512 byte hoặc `BLOCKSIZE`, làm tròn lên); in thêm dòng `total` khi ra terminal |
| `-t` | Sắp xếp theo thời gian sửa đổi, mới nhất trước, sau đó theo tên |
| `-u` | Dùng thời điểm truy cập cuối để sắp xếp (`-t`) và in ra (`-l`) |
| `-w` | In nguyên ký tự không in được (mặc định khi không ra terminal) |

Hành vi chung:

* Không có đối số thì liệt kê thư mục hiện tại.
* Các đối số không phải thư mục được in trước; tập tin và thư mục được sắp xếp
  riêng theo thứ tự từ điển. Khi có nhiều đối số, mỗi thư mục được mở đầu bằng
  một dòng `tên:`.
* Các tùy chọn ghi đè lẫn nhau theo nguyên tắc "tùy chọn đứng sau thắng":
  `-l`/`-n`, `-c`/`-u`, `-q`/`-w`, `-S`/`-t`, `-k`/`-h`.
* Biến môi trường `BLOCKSIZE` và `TZ` được tôn trọng.
* Mã thoát là `0` nếu thành công và `1` nếu có bất kỳ lỗi nào.

### Định dạng dài (`-l`)

Mỗi dòng gồm: chế độ tập tin, số liên kết, chủ sở hữu, nhóm, kích thước (byte),
ngày giờ (`Mon DD HH:MM` với tập tin gần đây, `Mon DD  YYYY` với tập tin cũ) và
tên. Ngoài ra:

* Chuỗi quyền gồm ký tự loại tập tin (`- d l b c s p w`, và `a`/`A` cho các trạng
  thái lưu trữ của NetBSD), `rwx` cho chủ sở hữu/nhóm/người khác, `s`/`S` cho
  set-user-ID và set-group-ID, `t`/`T` cho bit sticky.
* Liên kết mềm hiển thị dạng `tên -> đích`.
* Tập tin thiết bị hiển thị `major, minor` ở cột kích thước.
* Các cột được căn theo giá trị rộng nhất của từng cột.
* Dòng `total N` được in trước nội dung của mỗi thư mục.

## 5. Các trường hợp biên và tính ổn định

* Tập tin không tồn tại hoặc thư mục không đọc được sẽ in thông báo lỗi ra
  luồng lỗi chuẩn và chương trình vẫn tiếp tục với các đối số còn lại; mã thoát
  là 1.
* Thư mục không đọc được vẫn in dòng tiêu đề `tên:` trước thông báo lỗi (giống
  `ls` của hệ thống); thư mục rỗng không in gì (không có `total 0`).
* Liên kết mềm bị hỏng, FIFO, tập tin thiết bị, tập tin set-id và sticky, tên
  tập tin chứa ký tự điều khiển (`-q` / `-w`).
* Tên tập tin rất dài (200 ký tự) và thư mục có 2000 tập tin (danh sách mục tự
  mở rộng khi cần).
* `--` kết thúc việc đọc tùy chọn; đối số rỗng (`ls ""`) là một lỗi.
* Mọi giá trị trả về của `malloc`, `realloc`, `stat`, `lstat`, `opendir`,
  `readdir`, `readlink` đều được kiểm tra; không giới hạn cứng số lượng mục;
  toàn bộ bộ nhớ được giải phóng.
* Liên kết mềm trong danh sách đối số không bị đi theo với `-d`, `-l`, `-n`
  hoặc `-F`, còn lại thì được đi theo (như `ls` của hệ thống).
* Lỗi ghi ra đầu ra chuẩn (ví dụ đầy đĩa) được phát hiện và báo cáo.

## 6. Các quyết định thiết kế

Những chỗ trang manual không nói hoặc nói chưa chính xác, chương trình theo hành
vi của `ls` thật trên NetBSD, đã được kiểm chứng bằng script kiểm thử:

* `-f` đồng thời ngầm bật `-a`.
* `-d` luôn thắng `-R`, bất kể thứ tự (manual ghi là hai tùy chọn ghi đè lẫn
  nhau).
* Với `-h`, `-s` in kích thước theo byte ở dạng ngắn, và dung lượng đĩa đã dùng
  ở dạng dài; dòng `total` là tổng kích thước theo byte.
* Với `-R` và chỉ một đối số, thư mục đầu tiên không có dòng tiêu đề, chỉ các
  thư mục con mới có.
* `-S` và `-t` ghi đè lẫn nhau (cái sau thắng); các mục bằng khóa sắp xếp thì
  xếp theo tên, và `-r` đảo ngược toàn bộ thứ tự.

## 7. Kiểm thử

Lệnh `make test` chạy `test.sh`. Script này dựng một cây thư mục thử (tập tin
thường, tập tin ẩn, thư mục, liên kết mềm, một liên kết hỏng, FIFO, tập tin
set-uid, thư mục sticky, thư mục không đọc được, tên 200 ký tự, thư mục có 2000
tập tin) và chạy 81 trường hợp qua cả `./myls` lẫn `/bin/ls`: từng tùy chọn đơn
lẻ, các tổ hợp tùy chọn, các kiểu đối số, trường hợp biên, tùy chọn không hợp
lệ, và thư mục mặc định. Đầu ra (kể cả thông báo lỗi) và mã thoát phải hoàn
toàn giống nhau.

Kết quả trên NetBSD 10.1:

```
passed: 81   failed: 0
```

Chương trình cũng được biên dịch với `-fsanitize=undefined` và chạy với
`MALLOC_OPTIONS=AJ` (dừng khi dùng bộ cấp phát sai, lấp dữ liệu rác); cả 81
trường hợp vẫn đạt và không có lỗi nào được báo cáo. Không dùng được
AddressSanitizer vì nó từ chối chạy trên bản cài NetBSD này khi ASLR đang bật.

## 8. Hạn chế đã biết

* Chương trình viết cho NetBSD (`st_mtimespec`, `S_ISWHT`), không nhằm biên dịch
  trên hệ thống khác mà không chỉnh sửa.
* Sắp xếp dùng `strcmp` (theo thứ tự byte), như trong locale C; không có so sánh
  theo ngôn ngữ.
* Tên chủ sở hữu hoặc nhóm dài hơn 63 ký tự sẽ bị cắt ở định dạng dài.
* Đầu ra là một cột (manual mô tả mỗi mục một dòng); không có bố cục nhiều cột.

## 9. Tài liệu tham khảo

* Trang manual `ls(1)`, NetBSD 10.1 (đi kèm đề bài, tập tin `ls.pdf`)
* `stat(2)`, `lstat(2)`, `readlink(2)`, `opendir(3)`, `readdir(3)`,
  `getopt(3)`, `getpwuid(3)`, `getgrgid(3)`, `strftime(3)`, `qsort(3)`
