# redis-server
A C++ implementation for redis server

## How to Build

To build this Redis server implementation, follow these steps:

1. Ensure you have the following prerequisites installed:
   - C++ compiler with C++17 support (e.g., GCC 7+ or Clang 5+)
   - CMake (version 3.10 or higher)

2. Clone the repository:
   ```
   git clone https://github.com/yourusername/redis-server.git
   cd redis-server
   ```

3. Create a build directory and navigate to it:
   ```
   mkdir build
   cd build
   ```

4. Generate the build files using CMake:
   ```
   cmake ..
   ```

5. Build the project:
   ```
   make
   ```

6. After successful compilation, you'll find the executable in the `build` directory.

7. Run the server:
   ```
   ./server
   ```
   For all available options:
   ```
   ./server -h
   ```  
   
## FAQ

### Q: What Redis commands are supported by this implementation?
A: This implementation supports basic Redis commands such as PING, ECHO, GET, SET, CONFIG, KEYS, and INFO. For a complete list of supported commands, please refer to the `initCmdsLUT` function in the `src/RedisServer.cpp` file.

### Q: Does this implementation support Redis replication?
A: Yes, this implementation includes basic support for Redis replication. It can be configured as a replica and connect to a master server. The replication functionality can be found in the `handShakeMaster` method of the `Server` class.

### Q: How does this implementation handle command execution?
A: Commands are processed using a lookup table (LUT) that maps command names to their corresponding handler functions. When a command is received, the server looks up the appropriate handler in the `cmdsLUT` and executes it.

### Q: Is there support for key expiration?
A: Yes, this implementation supports key expiration. When setting a key, an optional expiry time can be provided. The `getValue` method checks for key expiration before returning a value.

### Q: How can I contribute to this project?
A: Contributions are welcome! Please fork the repository, make your changes, and submit a pull request. Make sure to follow the existing code style and include appropriate tests for new features.

### Q: Is this implementation suitable for production use?
A: Everything is production ready if you are brave enough. 



