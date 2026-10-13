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
for o in -la -lh -ln -li -ls -lsk -lsh -sk -sh -lF -lS -lSr -ltr -lc -lu \
    -ltc -ltu -lisaF -lAF -laR -lR -Rd -dR -lq -lw -lkh -lhk; do
	check $o "$DIR"
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

echo "--- no operand (current directory)"
cd "$DIR" || exit 1
check
check -la
check -R
cd "$ROOT" || exit 1

# ---------------------------------------------------------------------
# Summary and cleanup
# ---------------------------------------------------------------------
rm -rf "$WORK"
echo
echo "passed: $pass   failed: $fail"
[ $fail -eq 0 ]
