1. ##### x-monitor

   - 依赖

     ```
     dnf install libev.x86_64 libev-devel.x86_64
     wget https://ftp.gnu.org/gnu/nettle/nettle-3.7.tar.gz
     wget https://ftp.gnu.org/gnu/libidn/libidn2-2.3.2.tar.gz
     git clone https://github.com/libffi/libffi.git
     wget https://ftp.gnu.org/gnu/libtasn1/libtasn1-4.18.0.tar.gz
     wget https://ftp.gnu.org/gnu/libunistring/libunistring-1.0.tar.gz
     wget https://github.com/p11-glue/p11-kit/archive/refs/tags/0.24.0.tar.gz
     
     ./configure --prefix=/usr --enable-static #编译静态库
     https://www.gnutls.org/download.html
     https://www.gnu.org/software/libunistring/#TOCdownloading
     ```

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

   - 查看状态

     ```
     top -d 1 -p  `pidof x-monitor`
     pidstat -r -u -t -p  `pidof x-monitor` 1 10000
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

5. ##### simplepattern_test

   - 编译

     ```
     make simplepattern_test VERBOSE=1
     ```

   - 运行

     ```
     bin/simplepattern_test ../cli/simplepattern_test/log.cfg
     ```

   - 检查是否有内存泄露

     ```
     valgrind --tool=memcheck --leak-check=full bin/simplepattern_test ../cli/simplepattern_test/log.cfg
     ```

6. ##### x-monitor的性能分析

   1. 整个系统的cpu实时开销排序

      ```none
      perf top --sort cpu
      ```

   2. 进程采样

      ```
      perf record -F 99 -p 62275 -e cpu-clock -ag --call-graph dwarf sleep 10
      ```

      -F 99：每秒采样的99次

      -g：记录调用堆栈

   3. 采样结果

      ```
      perf report -n
      ```

      生成报告预览

      ```
      perf report -n --stdio
      ```

      生成详细的报告

      ```
      perf script > out.perf
      ```

      dump出perf.data的内容

   4. 生成svg图

      ```
      yum -y install perl-open.noarch
      perf script -i perf.data &> perf.unfold
      stackcollapse-perf.pl perf.unfold &> perf.folded
      flamegraph.pl perf.folded > perf.svg
      ```

7. ##### 监控指标

   1. 进程的打开文件句柄数量



