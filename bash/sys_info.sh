lscpu | grep -E '^(Model name|Socket\(s\)|Core\(s\) per socket|Thread\(s\) per core|CPU\(s\)):'
  echo "System:    $(cat /sys/class/dmi/id/sys_vendor) $(cat /sys/class/dmi/id/product_name)"
  echo "BIOS:      $(cat /sys/class/dmi/id/bios_version) ($(cat /sys/class/dmi/id/bios_date))"
  echo "OS:        $(. /etc/os-release; echo $PRETTY_NAME)"
  echo "Kernel:    $(uname -r)"
  echo "Host:      $(hostname)"
  echo "DRAM SIZE: $(awk '/MemTotal/ {printf "%.1f GiB\n", $2/1024/1024}' /proc/meminfo)"
  echo "DISK SIZE: $(df -h --output=used,size / | tail -1 | awk '{print $1" used / "$2" total"}')"
