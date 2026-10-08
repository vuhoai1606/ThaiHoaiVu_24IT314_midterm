# BÁO CÁO BÀI TẬP GIỮA KỲ: HIỆN THỰC LỆNH LS(1)
**Môn học:** Lập trình hệ thống (System Programming)  
**Sinh viên:** Thái Hoài Vũ  
**Mã số sinh viên (MSSV):** 24IT314  
**GitHub Repository:** `https://github.com/<your-username>/ThaiHoaiVu_24IT314_midterm`

---

## 1. Giới thiệu dự án (Project Overview)
Dự án nhằm mục đích hiện thực lại lệnh `ls(1)` trong hệ điều hành UNIX/NetBSD từ đầu bằng ngôn ngữ C chuẩn C99/POSIX, dựa trên tài liệu **NetBSD 10.1 General Commands Manual**.

Chương trình được thiết kế theo kiến trúc module hóa chặt chẽ, chia tách thành các file mã nguồn `.c` và file tiêu đề `.h` độc lập, tuân thủ tiêu chuẩn lập trình hệ thống nghiêm ngặt (`-Wall -Wextra -Werror -pedantic -std=c99`), xử lý đầy đủ các trường hợp biên và không bị rò rỉ bộ nhớ hay lỗi segmentation fault.

---

## 2. Cấu trúc thư mục và tổ chức mã nguồn (Project Architecture)

```text
ThaiHoaiVu_24IT314_midterm/
├── Makefile                # File cấu hình biên dịch tự động
├── README.md               # Báo cáo chi tiết dự án & hướng dẫn sử dụng
├── .gitignore              # Loại bỏ các file nhị phân và file đối tượng (.o)
├── include/                # Thư mục chứa các file tiêu đề (.h)
│   ├── compat.h            # Định nghĩa tương thích POSIX/NetBSD và cross-platform
│   ├── options.h           # Định nghĩa cấu trúc lưu cờ và hàm parse tham số CLI
│   ├── file_entry.h        # Cấu trúc lưu thông tin metadata của file và quản lý bộ nhớ
│   ├── sort.h              # Các giải thuật so sánh và sắp xếp file entries
│   ├── display.h           # Định dạng và hiển thị kết quả ra màn hình (short/long format)
│   └── traverse.h          # Thuật toán duyệt thư mục, đệ quy (-R) và xử lý đối số
└── src/                    # Thư mục chứa các file mã nguồn (.c)
    ├── main.c              # Hàm main: khởi tạo, điều phối luồng và trả mã lỗi thoát
    ├── options.c           # Phân tích cờ dòng lệnh (getopt) và áp dụng override rules
    ├── file_entry.c        # Khởi tạo entry, gọi lstat, đọc symlink và giải phóng bộ nhớ
    ├── sort.c              # Triển khai sắp xếp theo tên, size, time và reverse
    ├── display.c           # Format mode string (10 ký tự), human size, date, căn cột -l
    └── traverse.c          # Đọc thư mục (opendir/readdir), đệ quy và phân loại operands
```

---

## 3. Danh sách tính năng và các cờ (Options) đã hiện thực

Chương trình hiện thực toàn bộ các cờ được quy định trong tài liệu manual page `ls [-AacdFfhiklnqRrSstuw] [file ...]`:

| Cờ (Flag) | Chức năng mô tả | Hiện thực kỹ thuật |
| :---: | :--- | :--- |
| **`-a`** | Liệt kê tất cả các file bao gồm file ẩn bắt đầu bằng dấu `.`, kể cả `.` và `..`. | Đọc toàn bộ entry từ `readdir()`. |
| **`-A`** | Liệt kê tất cả file ẩn ngoại trừ `.` và `..`. Tự động bật với super-user. | Bỏ qua mục có tên chính xác là `.` hoặc `..`. |
| **`-c`** | Dùng thời gian thay đổi trạng thái file (`ctime`) cho việc sắp xếp (`-t`) hoặc in (`-l`). | Sử dụng `st.st_ctime` thay cho `mtime`. |
| **`-d`** | Liệt kê thư mục như một file thông thường (không duyệt vào nội dung thư mục). | Không gọi `opendir()` đối với thư mục đối số. |
| **`-F`** | Thêm ký tự chỉ thị: `/` (thư mục), `*` (file thực thi), `@` (symlink), `=` (socket), `\|` (FIFO), `%` (whiteout). | Kiểm tra bitmask `st_mode` qua `S_ISDIR`, `S_ISLNK`, `st_mode & 0111`,... |
| **`-f`** | Xuất kết quả theo thứ tự đọc trong thư mục, **không sắp xếp** (đồng thời bật `-a`). | Bỏ qua bước gọi `qsort()` và bật cờ hiển thị tất cả. |
| **`-h`** | Định dạng dung lượng dễ đọc cho con người (B, K, M, G, T) khi đi kèm `-l` hoặc `-s`. | Hàm `format_human_size` chia lũy thừa 1024 với tiền tố chuẩn. |
| **`-i`** | In số inode (file serial number) của mỗi file. | In giá trị `st_ino`. |
| **`-k`** | Định dạng kích thước/block theo đơn vị Kilobyte (1024 bytes). Ghi đè cờ `-h`. | Quy đổi block và kích thước theo đơn vị 1024 bytes. |
| **`-l`** | Hiển thị định dạng danh sách dài (long format): mode, links, owner, group, size, time, name. | Tính toán độ rộng lớn nhất của từng cột để căn chỉnh đều hàng, in `total <blocks>`. |
| **`-n`** | Tương tự `-l`, nhưng hiển thị UID và GID dạng số thay vì tra cứu tên user/group. | In trực tiếp `st_uid` và `st_gid`. |
| **`-q`** | In các ký tự không in được thành ký tự `?` (mặc định khi xuất ra terminal). | Dùng `isprint()` kiểm tra từng ký tự trong tên file. |
| **`-w`** | In thô ký tự không in được (mặc định khi chuyển hướng pipe/file). | Xuất nguyên vẹn chuỗi byte. |
| **`-R`** | Liệt kê đệ quy tất cả các thư mục con bắt gặp. | Duyệt cây thư mục đệ quy, in header `path:` cho mỗi thư mục. |
| **`-r`** | Đảo ngược thứ tự sắp xếp (reverse sort). | Nhân hệ số `-1` vào kết quả của hàm so sánh `qsort()`. |
| **`-S`** | Sắp xếp theo kích thước file giảm dần (lớn nhất lên trước). | So sánh `st_size` của 2 file entries. |
| **`-s`** | Hiển thị số block hệ thống tập tin thực tế mà file chiếm dụng. | Tính toán dựa trên `st_blocks` và biến `BLOCKSIZE`. |
| **`-t`** | Sắp xếp theo thời gian sửa đổi gần nhất lên trước. | So sánh timestamp (`mtime`, `ctime` nếu có `-c`, `atime` nếu có `-u`). |
| **`-u`** | Dùng thời gian truy cập gần nhất (`atime`) cho việc sắp xếp (`-t`) hoặc in (`-l`). | Sử dụng `st.st_atime` thay cho `mtime`. |

### Quy tắc ghi đè cờ (Override Rules)
Tuân thủ nghiêm ngặt đặc tả NetBSD Manual:
- `-w` và `-q`: Cờ xuất hiện sau cùng sẽ quyết định định dạng xuất ký tự.
- `-l` và `-n`: Cờ xuất hiện sau cùng sẽ quyết định định dạng hiển thị tên/ID.
- `-c` và `-u`: Cờ xuất hiện sau cùng sẽ quyết định mốc thời gian được sử dụng.
- `-h` và `-k`: Cờ xuất hiện sau cùng sẽ ghi đè cờ phía trước.
- `-R` và `-d`: Cờ `-d` tắt đệ quy; cờ xuất hiện sau cùng sẽ có hiệu lực.

### Xử lý biến môi trường
- `BLOCKSIZE`: Nếu biến môi trường `BLOCKSIZE` được thiết lập và không có `-h` hoặc `-k`, đơn vị khối cho `-s` và dòng `total` sẽ được tính theo giá trị của biến này.

### Xử lý đối số (Operands)
- Nếu không có đối số nào được truyền vào: mặc định liệt kê thư mục hiện tại (`.`).
- Nếu có nhiều đối số:
  1. Phân loại các đối số thành 2 nhóm: **Non-directory** (file thông thường) và **Directory** (thư mục).
  2. Sắp xếp và hiển thị nhóm **Non-directory** trước.
  3. Duyệt và hiển thị từng thư mục trong nhóm **Directory** (in tiêu đề `dir:` và ngắt dòng phân cách).
  4. Nếu một đối số không tồn tại hoặc lỗi quyền truy cập: in thông báo lỗi chuẩn `ls: <operand>: <thông_báo_lỗi>` ra `stderr` và tiếp tục duyệt các đối số còn lại, kết thúc chương trình với mã thoát > 0.

---

## 4. Hướng dẫn biên dịch và chạy chương trình (Build & Run)

### Biên dịch:
Mở terminal trong thư mục dự án và chạy:
```bash
make
```
Lệnh trên sẽ tự động biên dịch tất cả các file nguồn với các cờ cảnh báo nghiêm ngặt và tạo ra file nhị phân `ls` (hoặc `ls.exe` trên Windows).

Để xóa các file đối tượng (`.o`) và file thực thi đã biên dịch:
```bash
make clean
```

### Chạy chương trình:
```bash
# Liệt kê thư mục hiện tại:
./ls

# Liệt kê đầy đủ định dạng dài:
./ls -l

# Liệt kê file ẩn và hiển thị kích thước dạng human-readable:
./ls -lah

# Hiển thị số inode, số block và ký tự phân loại file:
./ls -isF

# Sắp xếp theo dung lượng giảm dần:
./ls -lS

# Sắp xếp theo thời gian mới nhất, đảo ngược thứ tự:
./ls -ltr

# Duyệt đệ quy toàn bộ thư mục:
./ls -R

# Liệt kê chính thư mục mà không duyệt nội dung bên trong:
./ls -ld include src
```

---

## 5. Minh chứng kiểm thử (Verification & Test Cases)

1. **Kiểm tra hiển thị mặc định:**
   ```bash
   $ ./ls
   Makefile
   README.md
   include
   src
   ```
2. **Kiểm tra cờ `-F` (chỉ thị loại file):**
   ```bash
   $ ./ls -F
   Makefile
   README.md
   include/
   ls*
   src/
   ```
3. **Kiểm tra cờ `-l` và `-h`:**
   ```bash
   $ ./ls -lh
   total 215K
   -rw-rw-rw- 1 user0 group0  542B Oct  8 23:14 Makefile
   -rw-rw-rw- 1 user0 group0  6.5K Oct  8 23:20 README.md
   drwxrwxrwx 1 user0 group0    0B Oct  8 23:06 include
   -rwxrwxrwx 1 user0 group0  105K Oct  8 23:14 ls
   drwxrwxrwx 1 user0 group0    0B Oct  8 23:11 src
   ```
4. **Kiểm tra xử lý lỗi khi file không tồn tại:**
   ```bash
   $ ./ls non_existent_file Makefile
   Makefile
   ls: non_existent_file: No such file or directory
   $ echo $?
   1
   ```

---

## 6. Kết luận & Tự đánh giá
- Chương trình hoàn thành đầy đủ 100% các yêu cầu trong đề cương và đặc tả NetBSD 10.1 `ls(1)`.
- Không xảy ra lỗi rò rỉ bộ nhớ hay lỗi segmentation fault khi kiểm tra với các cây thư mục phức tạp và các trường hợp biên.
- Mã nguồn được cấu trúc rõ ràng, chú thích đầy đủ theo chuẩn học thuật.
