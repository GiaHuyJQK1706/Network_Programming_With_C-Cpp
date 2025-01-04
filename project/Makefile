# Biến cấu hình
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
SRC_DIR = src
UTILS_DIR = $(SRC_DIR)/utils
BUILD_DIR = build
DB_DIR = db
ARCHIVE_DIR = archive
EXECUTABLE_SERVER = server
EXECUTABLE_CLIENT = client

# Danh sách tệp nguồn
SERVER_SRC = $(SRC_DIR)/server.cpp \
             $(UTILS_DIR)/file_manager.cpp \
             $(UTILS_DIR)/auth_manager.cpp \
             $(UTILS_DIR)/socket_handler.cpp

CLIENT_SRC = $(SRC_DIR)/client.cpp \
             $(UTILS_DIR)/socket_handler.cpp

# Tệp đối tượng (object files)
SERVER_OBJ = $(SERVER_SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
CLIENT_OBJ = $(CLIENT_SRC:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Quy tắc mặc định
all: directories $(EXECUTABLE_SERVER) $(EXECUTABLE_CLIENT)

# Biên dịch server
$(EXECUTABLE_SERVER): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Biên dịch client
$(EXECUTABLE_CLIENT): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Quy tắc để tạo thư mục build nếu chưa tồn tại
directories:
	mkdir -p $(BUILD_DIR)/utils

# Quy tắc để tạo tệp đối tượng
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: $(UTILS_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Quy tắc dọn dẹp
clean:
	rm -rf $(BUILD_DIR) $(EXECUTABLE_SERVER) $(EXECUTABLE_CLIENT)
