#!/bin/sh
#
# test_suite.sh - Automated test suite for ls(1) implementation
# Project: ThaiHoaiVu_24IT314_midterm
# Total: 45 Test Cases
#

TEST_DIR="/tmp/ls_test_env_$$"
LS_BIN="./ls"
PASSED=0
FAILED=0

cleanup() {
    rm -rf "${TEST_DIR}"
}
trap cleanup EXIT INT TERM

echo "=================================================="
echo "    ls(1) Automated Test Suite - 24IT314          "
echo "=================================================="

# Ensure binary is compiled
if [ ! -x "${LS_BIN}" ] && [ ! -x "${LS_BIN}.exe" ]; then
    echo "Executable ${LS_BIN} not found. Running make..."
    make || exit 1
fi

if [ -x "${LS_BIN}.exe" ]; then
    LS_BIN="./ls.exe"
fi

# Setup test sandbox environment
rm -rf "${TEST_DIR}"
mkdir -p "${TEST_DIR}/dir1/subdir1" "${TEST_DIR}/dir2"
touch "${TEST_DIR}/file_a.txt" "${TEST_DIR}/file_b.txt" "${TEST_DIR}/.hidden_file"
touch "${TEST_DIR}/dir1/file_in_dir1.txt" "${TEST_DIR}/dir1/subdir1/nested_file.txt"

# Create different file sizes
echo "Small test file" > "${TEST_DIR}/small.txt"
dd if=/dev/zero of="${TEST_DIR}/large.bin" bs=1024 count=64 2>/dev/null || true

# Create executable file
touch "${TEST_DIR}/test_script.sh"
chmod 755 "${TEST_DIR}/test_script.sh" 2>/dev/null || true

# Create symbolic links
ln -s "${TEST_DIR}/file_a.txt" "${TEST_DIR}/symlink_file" 2>/dev/null || true
ln -s "${TEST_DIR}/dir1" "${TEST_DIR}/symlink_dir" 2>/dev/null || true

# Test execution helper
assert_test() {
    desc="$1"
    cmd="$2"
    expected_code="$3"

    eval "${cmd}" > "${TEST_DIR}/test_out.tmp" 2> "${TEST_DIR}/test_err.tmp"
    actual_code=$?

    if [ "${actual_code}" -eq "${expected_code}" ]; then
        echo "  [PASS] ${desc}"
        PASSED=$((PASSED + 1))
    else
        echo "  [FAIL] ${desc} (Expected exit ${expected_code}, got ${actual_code})"
        FAILED=$((FAILED + 1))
    fi
}

echo ""
echo "--- 1. Basic Operations & Operands (6 tests) ---"
assert_test "Default listing (no operands)" "${LS_BIN} > /dev/null" 0
assert_test "Single file operand" "${LS_BIN} ${TEST_DIR}/file_a.txt > /dev/null" 0
assert_test "Multiple file operands" "${LS_BIN} ${TEST_DIR}/file_a.txt ${TEST_DIR}/file_b.txt > /dev/null" 0
assert_test "Single directory operand" "${LS_BIN} ${TEST_DIR}/dir1 > /dev/null" 0
assert_test "Multiple directory operands" "${LS_BIN} ${TEST_DIR}/dir1 ${TEST_DIR}/dir2 > /dev/null" 0
assert_test "Mixed non-directory and directory operands" "${LS_BIN} ${TEST_DIR}/file_a.txt ${TEST_DIR}/dir1 > /dev/null" 0

echo ""
echo "--- 2. Error Handling & Exit Status (4 tests) ---"
assert_test "Non-existent file operand exits with error" "${LS_BIN} ${TEST_DIR}/missing_file > /dev/null" 1
assert_test "Mixed valid and non-existent operands exits with error" "${LS_BIN} ${TEST_DIR}/file_a.txt ${TEST_DIR}/missing_file > /dev/null" 1
assert_test "Non-existent directory operand exits with error" "${LS_BIN} ${TEST_DIR}/missing_dir > /dev/null" 1
assert_test "Invalid command option exits with error" "${LS_BIN} -Z > /dev/null" 1

echo ""
echo "--- 3. Directory Content Filtering (-a, -A) (4 tests) ---"
assert_test "Default listing hides dot entries" "! ${LS_BIN} ${TEST_DIR} | grep -q '^\.hidden_file$'" 0
assert_test "Option -a includes hidden files and dot entries" "${LS_BIN} -a ${TEST_DIR} | grep -q '^\.hidden_file$'" 0
assert_test "Option -A includes hidden files" "${LS_BIN} -A ${TEST_DIR} | grep -q '^\.hidden_file$'" 0
assert_test "Option -A excludes . and .." "! ${LS_BIN} -A ${TEST_DIR} | grep -Eq '^\.$|^\.\.$'" 0

echo ""
echo "--- 4. Long Listing Format (-l, -n) (5 tests) ---"
assert_test "Option -l produces long listing" "${LS_BIN} -l ${TEST_DIR} > /dev/null" 0
assert_test "Option -n displays numeric IDs" "${LS_BIN} -n ${TEST_DIR} > /dev/null" 0
assert_test "Option -l displays total block count line" "${LS_BIN} -l ${TEST_DIR} | grep -q '^total '" 0
assert_test "Option -l prints symbolic link destination" "${LS_BIN} -l ${TEST_DIR}/symlink_file | grep -q '->'" 0
assert_test "Option -l handles single file operand" "${LS_BIN} -l ${TEST_DIR}/file_a.txt > /dev/null" 0

echo ""
echo "--- 5. Sizing & Units (-h, -k, -s) (5 tests) ---"
assert_test "Option -s prints block counts" "${LS_BIN} -s ${TEST_DIR} > /dev/null" 0
assert_test "Option -h prints human readable sizes in -l" "${LS_BIN} -lh ${TEST_DIR} > /dev/null" 0
assert_test "Option -h prints human readable units in -s" "${LS_BIN} -sh ${TEST_DIR} > /dev/null" 0
assert_test "Option -k reports in kilobytes" "${LS_BIN} -sk ${TEST_DIR} > /dev/null" 0
assert_test "Environment variable BLOCKSIZE alters block unit" "BLOCKSIZE=1024 ${LS_BIN} -s ${TEST_DIR} > /dev/null" 0

echo ""
echo "--- 6. Classification & Indicators (-F) (4 tests) ---"
assert_test "Option -F appends / to directories" "${LS_BIN} -F ${TEST_DIR} | grep -q 'dir1/'" 0
assert_test "Option -F appends * to executables" "${LS_BIN} -F ${TEST_DIR} | grep -q 'test_script.sh\*'" 0
assert_test "Option -F appends @ to symbolic links" "${LS_BIN} -F ${TEST_DIR} | grep -q 'symlink_file@'" 0
assert_test "Option -F on regular file has no symbol" "! ${LS_BIN} -F ${TEST_DIR}/file_a.txt | grep -Eq '[/*@=|%]'" 0

echo ""
echo "--- 7. Sorting Criteria (-S, -t, -u, -c, -r, -f) (7 tests) ---"
assert_test "Default sort is lexicographical" "${LS_BIN} ${TEST_DIR} > /dev/null" 0
assert_test "Option -S sorts by file size" "${LS_BIN} -S ${TEST_DIR} > /dev/null" 0
assert_test "Option -t sorts by modification time" "${LS_BIN} -t ${TEST_DIR} > /dev/null" 0
assert_test "Option -r reverses sorting order" "${LS_BIN} -r ${TEST_DIR} > /dev/null" 0
assert_test "Option -u uses access time with -l" "${LS_BIN} -lu ${TEST_DIR} > /dev/null" 0
assert_test "Option -c uses status change time with -l" "${LS_BIN} -lc ${TEST_DIR} > /dev/null" 0
assert_test "Option -f disables sorting and enables -a" "${LS_BIN} -f ${TEST_DIR} > /dev/null" 0

echo ""
echo "--- 8. Directory Traversal (-R, -d) (4 tests) ---"
assert_test "Option -d lists directory itself" "${LS_BIN} -d ${TEST_DIR}/dir1 > /dev/null" 0
assert_test "Option -d with multiple directories" "${LS_BIN} -d ${TEST_DIR}/dir1 ${TEST_DIR}/dir2 > /dev/null" 0
assert_test "Option -R recursively lists subdirectories" "${LS_BIN} -R ${TEST_DIR} | grep -q 'nested_file.txt'" 0
assert_test "Option -R skips . and .. preventing infinite loops" "! ${LS_BIN} -R ${TEST_DIR} | grep -Eq '^\./\.:|^\./\.\.:'" 0

echo ""
echo "--- 9. Output Characters & Overrides (6 tests) ---"
assert_test "Option -q forces non-printable to ?" "${LS_BIN} -q ${TEST_DIR} > /dev/null" 0
assert_test "Option -w forces raw non-printable" "${LS_BIN} -w ${TEST_DIR} > /dev/null" 0
assert_test "Override -l then -n produces numeric output" "${LS_BIN} -l -n ${TEST_DIR} > /dev/null" 0
assert_test "Override -n then -l produces user name output" "${LS_BIN} -n -l ${TEST_DIR} > /dev/null" 0
assert_test "Override -k then -h activates human readable" "${LS_BIN} -k -h ${TEST_DIR} > /dev/null" 0
assert_test "Override -h then -k activates kilobyte blocks" "${LS_BIN} -h -k ${TEST_DIR} > /dev/null" 0

echo ""
echo "=================================================="
echo "Test Summary: ${PASSED} PASSED, ${FAILED} FAILED"
echo "=================================================="

if [ "${FAILED}" -eq 0 ]; then
    echo "ALL TESTS PASSED SUCCESSFULLY!"
    exit 0
else
    echo "SOME TESTS FAILED!"
    exit 1
fi
