1. 编译

   ```
   cmake3 ../ -DCMAKE_BUILD_TYPE=Debug -DSTATIC_LINKING=1 -DSTATIC_LIBC=1
   make x-monitor VERBOSE=1
   ```

2. 运行

   ```
   bin/x-monitor -c ../env/config/x-monitor.cfg
   ```

3. 停止

   ```
   kill -15 `pidof x-monitor`
   ```

   