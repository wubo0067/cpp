1. ##### x-monitor

   - 编译

   ```
   cmake3 ../ -DCMAKE_BUILD_TYPE=Debug -DSTATIC_LINKING=1 -DSTATIC_LIBC=1
   make x-monitor VERBOSE=1
   ```

   - 运行

   ```
   bin/x-monitor -c ../env/config/x-monitor.cfg
   ```

   - 停止

   ```
   kill -15 `pidof x-monitor`
   ```

2. ##### proc_file

   - 编译

     ```
     make procfile_cli VERBOSE=1
     ```

   - 运行

     ```
     bin/procfile_cli ../cli/procfile_cli/log.cfg /proc/diskstats 10
     bin/procfile_cli ../cli/procfile_cli/log.cfg /proc/meminfo 10
     ```

3. ##### perf_event_stack

   - 编译

     ```
     make perf_event_stack_cli VERBOSE=1
     ```

   - 运行

4. ##### proto_statistics_cli

   - 编译

     ```
     make proto_statistics_cli VERBOSE=1
     ```

   - 查看map数据

     ```
     bpftool map dump name proto_countmap
     ```

   - 运行

     ```
     bin/proto_statistics_cli ../collectors/ebpf/kernel/xmbpf_proto_statistics_kern.5.12.o eth0
     ```

     




