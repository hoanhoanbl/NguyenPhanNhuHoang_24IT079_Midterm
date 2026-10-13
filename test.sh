#!/bin/sh
#
# test.sh - compare ./myls with the system ls (/bin/ls) on a directory
#           tree built for the purpose.  Usage: make test   (or: sh test.sh)
#
# Every case runs both programs with the same arguments and compares
# their output (with the program name normalised) and exit status.

ROOT=$(pwd)
MYLS=$ROOT/myls
REAL=/bin/ls

# One work directory holds the tree and the result files, so /tmp itself
# is not modified while the cases run (it is listed by one of them).
WORK=/tmp/myls_work.$$
DIR=$WORK/tree
MINE=$WORK/mine
REF=$WORK/real

pass=0
fail=0

# ---------------------------------------------------------------------
# Build the test tree with fixed modification times.
# ---------------------------------------------------------------------
mkdir -p "$DIR/sub/deep" "$DIR/empty" "$DIR/stick" || exit 1
echo hello > "$DIR/small"
head -c 5000 /dev/zero > "$DIR/big"
head -c 100 /dev/zero > "$DIR/mid"
: > "$DIR/.hidden"
: > "$DIR/sub/x"
: > "$DIR/sub/deep/z"
printf '#!/bin/sh\n' > "$DIR/prog"
chmod 4755 "$DIR/prog"
chmod 1777 "$DIR/stick"
ln -s nowhere "$DIR/broken"
ln -s small "$DIR/link"
ln -s sub "$DIR/dirlink"
mkfifo "$DIR/fifo"
: > "$DIR/tab	name"
touch -t 202001010000 "$DIR/old"
touch -t 202501011230 "$DIR/newer"

# Edge cases: unreadable directory, very long name, directory with many files.
mkdir "$DIR/noperm"
: > "$DIR/noperm/hidden_inside"
chmod 000 "$DIR/noperm"

longname=
n=0
while [ $n -lt 200 ]; do
	longname="${longname}a"
	n=$((n + 1))
done
: > "$DIR/$longname"

MANY=$WORK/many
mkdir "$MANY"
n=0
while [ $n -lt 2000 ]; do
	: > "$MANY/file$n"
	n=$((n + 1))
done

# Directory without symbolic links, with distinct access times.  Printing a
# symbolic link (readlink) updates its access time, which would make
# -u results depend on which program runs first.
PLAIN=$WORK/plain
mkdir "$PLAIN"
: > "$PLAIN/one"
: > "$PLAIN/two"
: > "$PLAIN/three"
touch -a -t 202001010000 "$PLAIN/one"
touch -a -t 202301010000 "$PLAIN/two"
touch -a -t 202401010000 "$PLAIN/three"

# ---------------------------------------------------------------------
# check <args...>: run both programs and compare.
# ---------------------------------------------------------------------
check()
{
	"$MYLS" "$@" > "$MINE" 2>&1
	mine_status=$?
	"$REAL" "$@" > "$REF" 2>&1
	real_status=$?

	# Error messages start with the program name: make them equal.
	sed 's/^myls:/ls:/' "$MINE" > "$MINE.n"

	if [ $mine_status -eq $real_status ] && cmp -s "$MINE.n" "$REF"; then
		pass=$((pass + 1))
		echo "PASS  ls $*"
	else
		fail=$((fail + 1))
		echo "FAIL  ls $*   (exit: mine=$mine_status real=$real_status)"
		diff "$MINE.n" "$REF" | head -8
	fi
}

# ---------------------------------------------------------------------
# Test cases
# ---------------------------------------------------------------------
echo "--- single options on a directory"
check "$DIR"
for o in -a -A -f -r -t -S -c -u -F -i -s -k -h -l -n -q -w -R -d; do
	check $o "$DIR"
done

echo "--- combined options"
for o in -la -lh -ln -li -ls -lsk -lsh -sk -sh -lF -lS -lSr -ltr -lc \
    -ltc -lisaF -lAF -laR -lR -Rd -dR -lq -lw -lkh -lhk; do
	check $o "$DIR"
done

echo "--- access time (directory without symbolic links)"
for o in -u -lu -ltu -ltur -lSu -tu; do
	check $o "$PLAIN"
done

echo "--- operands"
check "$DIR/small" "$DIR/sub"
check "$DIR/sub" "$DIR/empty"
check "$DIR/nofile" "$DIR"
check "$DIR/nofile"
check -l "$DIR/small" "$DIR/big" "$DIR/old"
check -d "$DIR" "$DIR/sub"
check -ld "$DIR/dirlink"
check -l "$DIR/dirlink"
check "$DIR/dirlink"
check -F "$DIR/dirlink"
check -ld /tmp
check -l /dev/null /dev/tty
check -R "$DIR/sub"
check -R "$DIR/empty" "$DIR/sub"

echo "--- edge cases"
check "$DIR/noperm"
check -l "$DIR/noperm"
check -R "$DIR"
check ""
check -- "$DIR/small"
check -l -- "-notanoption"
check "$DIR/sub" "$DIR/noperm" "$DIR/nofile"
check -l "$DIR/$longname"
check -la "$MANY"
check -lSr "$MANY"
check -t "$MANY"
check -R "$MANY"

echo "--- invalid option (usage message and exit status)"
"$MYLS" -z > "$MINE" 2>&1
if [ $? -eq 1 ]; then
	pass=$((pass + 1))
	echo "PASS  ls -z  (exit status 1)"
else
	fail=$((fail + 1))
	echo "FAIL  ls -z  (exit status should be 1)"
fi

echo "--- no operand (current directory)"
cd "$DIR" || exit 1
check
check -la
check -R
cd "$ROOT" || exit 1

# ---------------------------------------------------------------------
# Summary and cleanup
# ---------------------------------------------------------------------
chmod 755 "$DIR/noperm"
rm -rf "$WORK"
echo
echo "passed: $pass   failed: $fail"
[ $fail -eq 0 ]
