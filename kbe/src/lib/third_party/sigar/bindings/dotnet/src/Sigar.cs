/*
 * Copyright (c) 2004-2006 Hyperic, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

using System;
using System.IO;
using System.Collections;
using System.Runtime.InteropServices;
using System.Text;

namespace Hyperic.Sigar {
    public class Sigar {
        internal const int OK = 0;
        internal const int SIGAR_START_ERROR = 20000;
        internal const int SIGAR_ENOTIMPL = (SIGAR_START_ERROR + 1);

        public const String NULL_HWADDR = "00:00:00:00:00:00";

        public const int IFF_UP = 0x1;
        public const int IFF_BROADCAST = 0x2;
        public const int IFF_DEBUG = 0x4;
        public const int IFF_LOOPBACK = 0x8;
        public const int IFF_POINTOPOINT = 0x10;
        public const int IFF_NOTRAILERS = 0x20;
        public const int IFF_RUNNING = 0x40;
        public const int IFF_NOARP = 0x80;
        public const int IFF_PROMISC = 0x100;
        public const int IFF_ALLMULTI = 0x200;
        public const int IFF_MULTICAST = 0x800;

        internal const int FS_NAME_LEN = 64;

        //cp sigar-x86-winnt.dll sigar.dll
        //ln -s libsigar-x86-linux.so libsigar.so
        //XXX can we determine the real name at runtime?
        internal const string LIBSIGAR = "sigar";

        internal HandleRef sigar;

        [DllImport(LIBSIGAR)]
        private static extern int sigar_open(ref IntPtr sigar);

        [DllImport(LIBSIGAR)]
        private static extern int sigar_close(IntPtr sigar);

        public Sigar() {
            IntPtr handle = IntPtr.Zero;
            sigar_open(ref handle);
            this.sigar = new HandleRef(this, handle);
        }

        [DllImport(Sigar.LIBSIGAR)]
        private static extern void
        sigar_format_size(long size, StringBuilder buffer);

        public static string FormatSize(long size) {
            StringBuilder buffer = new StringBuilder(56);
            sigar_format_size(size, buffer);
            return buffer.ToString();
        }

        public Mem Mem() {
            return Hyperic.Sigar.Mem.NativeGet(this);
        }

        public Swap Swap() {
            return Hyperic.Sigar.Swap.NativeGet(this);
        }

        public Cpu Cpu() {
            return Hyperic.Sigar.Cpu.NativeGet(this);
        }

        public CpuInfo[] CpuInfoList() {
            return Hyperic.Sigar.CpuInfoList.NativeGet(this);
        }

        public FileSystem[] FileSystemList() {
            return Hyperic.Sigar.FileSystemList.NativeGet(this);
        }

        public FileSystemUsage FileSystemUsage(string dirname) {
            return Hyperic.Sigar.FileSystemUsage.NativeGet(this, dirname);
        }

        public String[] NetInterfaceList() {
            return Hyperic.Sigar.NetInterfaceList.NativeGet(this);
        }

        public NetInterfaceConfig NetInterfaceConfig(string name) {
            return Hyperic.Sigar.NetInterfaceConfig.NativeGet(this, name);
        }

        public NetInterfaceStat NetInterfaceStat(string name) {
            return Hyperic.Sigar.NetInterfaceStat.NativeGet(this, name);
        }

        ~Sigar() {
            sigar_close(this.sigar.Handle);
        }

        internal static IntPtr incrementIntPtr(IntPtr ptr, int size) {
            Int32 x = (Int32)ptr;
            x += size;
            return (IntPtr)x;
        }

        internal static SigarException FindException(Sigar sigar, int errno) {
            switch (errno) {
              case SIGAR_ENOTIMPL:
                return new SigarNotImplementedException(sigar, errno);
              default:
                return new SigarException(sigar, errno);
            }
        }
    }

    public class SigarException : Exception {
        Sigar sigar;
        int errno;

        public SigarException(Sigar sigar, int errno) : base() {
            this.sigar = sigar;
            this.errno = errno;
        }

        [DllImport(Sigar.LIBSIGAR)]
        private static extern string
        sigar_strerror(IntPtr sigar, int errno);

        public override string Message {
            get {
                return sigar_strerror(this.sigar.sigar.Handle, this.errno);
            }
        }
    }

    public class SigarNotImplementedException : SigarException {
        public SigarNotImplementedException(Sigar sigar, int errno) :
            base(sigar, errno) { }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Mem {
        public readonly long Ram;
        public readonly long Total;
        public readonly long Used;
        public readonly long Free;
        public readonly long Shared;
        public readonly long ActualFree;
        public readonly long ActualUsed;

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_mem_get(IntPtr sigar, IntPtr mem);

        internal static Mem NativeGet(Sigar sigar) {
            Type type = typeof(Mem);
            //sigar_mem_t *ptr = malloc(sizeof(*ptr))
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(type));

            int status = sigar_mem_get(sigar.sigar.Handle, ptr);

            if (status != Sigar.OK) {
                Marshal.FreeHGlobal(ptr);
                throw Sigar.FindException(sigar, status);
            }

            //memcpy(ptr, this, sizeof(this))
            Mem mem = (Mem)Marshal.PtrToStructure(ptr, type);
            Marshal.FreeHGlobal(ptr);
            return mem;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Swap {
        public readonly long Total;
        public readonly long Used;
        public readonly long Free;

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_swap_get(IntPtr sigar, IntPtr swap);

        internal static Swap NativeGet(Sigar sigar) {
            Type type = typeof(Swap);
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(type));

            int status = sigar_swap_get(sigar.sigar.Handle, ptr);

            if (status != Sigar.OK) {
                Marshal.FreeHGlobal(ptr);
                throw Sigar.FindException(sigar, status);
            }

            Swap swap = (Swap)Marshal.PtrToStructure(ptr, type);
            Marshal.FreeHGlobal(ptr);

            return swap;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Cpu {
        public readonly long User;
        public readonly long Sys;
        private readonly long NA_Nice;
        public readonly long Idle;
        private readonly long NA_Wait;
        public readonly long Total;

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_cpu_get(IntPtr sigar, IntPtr cpu);

        internal static Cpu NativeGet(Sigar sigar) {
            Type type = typeof(Cpu);
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(type));

            int status = sigar_cpu_get(sigar.sigar.Handle, ptr);

            if (status != Sigar.OK) {
                Marshal.FreeHGlobal(ptr);
                throw Sigar.FindException(sigar, status);
            }

            Cpu cpu = (Cpu)Marshal.PtrToStructure(ptr, type);
            Marshal.FreeHGlobal(ptr);

            return cpu;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct CpuInfo {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=128)]
        public readonly string Vendor; //char[128]
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=128)]
        public readonly string Model; //char[128]
        public readonly int Mhz;
        private readonly long CacheSize; //XXX not implemented
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct CpuInfoList {
        private readonly uint Number; //sizeof(unsigned long) == 4
        private readonly uint size;
        private readonly IntPtr data;

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_cpu_info_list_get(IntPtr sigar, IntPtr cpu_infos);

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_cpu_info_list_destroy(IntPtr sigar, IntPtr cpu_infos);

        internal static CpuInfo[] NativeGet(Sigar sigar) {
            Type type = typeof(CpuInfoList);
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(type));

            int status = sigar_cpu_info_list_get(sigar.sigar.Handle, ptr);

            if (status != Sigar.OK) {
                Marshal.FreeHGlobal(ptr);
                throw Sigar.FindException(sigar, status);
            }

            CpuInfoList infosPtr =
                (CpuInfoList)Marshal.PtrToStructure(ptr, type);

            CpuInfo[] infos = new CpuInfo[infosPtr.Number];

            IntPtr eptr = infosPtr.data;
            int size = Marshal.SizeOf(infos[0]);

            for (int i=0; i<infosPtr.Number; i++) {
                infos[i] =
                    (CpuInfo)Marshal.PtrToStructure(eptr, typeof(CpuInfo));

                //eptr += sizeof(sigar_cpu_info_t);
                eptr = Sigar.incrementIntPtr(eptr, size);
            }

            sigar_cpu_info_list_destroy(sigar.sigar.Handle, ptr);

            Marshal.FreeHGlobal(ptr);

            return infos;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct FileSystem {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=Sigar.FS_NAME_LEN)]
        public readonly string DirName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=Sigar.FS_NAME_LEN)]
        public readonly string DevName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=Sigar.FS_NAME_LEN)]
        public readonly string TypeName;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=Sigar.FS_NAME_LEN)]
        public readonly string SysTypeName;
        public readonly int Type;
        public readonly uint Flags;
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct FileSystemList {
        private readonly uint Number;
        private readonly uint size;
        private readonly IntPtr data;

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_file_system_list_get(IntPtr sigar, IntPtr fslist);

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_file_system_list_destroy(IntPtr sigar, IntPtr fslist);

        internal static FileSystem[] NativeGet(Sigar sigar) {
            Type type = typeof(FileSystemList);
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(type));

            int status = sigar_file_system_list_get(sigar.sigar.Handle, ptr);

            if (status != Sigar.OK) {
                Marshal.FreeHGlobal(ptr);
                throw Sigar.FindException(sigar, status);
            }

            FileSystemList fsPtr =
                (FileSystemList)Marshal.PtrToStructure(ptr, type);

            FileSystem[] fs = new FileSystem[fsPtr.Number];

            IntPtr fptr = fsPtr.data;
            int size = Marshal.SizeOf(fs[0]);

            for (int i=0; i<fsPtr.Number; i++) {
                fs[i] =
                    (FileSystem)Marshal.PtrToStructure(fptr,
                                                       typeof(FileSystem));

                //fptr += sizeof(sigar_fs_t);
                fptr = Sigar.incrementIntPtr(fptr, size);
            }

            sigar_file_system_list_destroy(sigar.sigar.Handle, ptr);

            Marshal.FreeHGlobal(ptr);

            return fs;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct FileSystemUsage {
        public readonly long Total;
        public readonly long Free;
        public readonly long Avail;
        public readonly long Used;
        private readonly long NA_Files; //XXX not implemented
        private readonly long NA_FreeFiles;
        private readonly long DiskReads;
        private readonly long DiskWrites;
        private readonly long DiskWriteBytes;
        private readonly long DiskReadBytes;
        private readonly long DiskQueue;
        public readonly double UsePercent;

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_file_system_usage_get(IntPtr sigar, string dirname,
                                    IntPtr fsusage);

        internal static FileSystemUsage NativeGet(Sigar sigar,
                                                  string dirname) {
            Type type = typeof(FileSystemUsage);
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(type));

            int status = sigar_file_system_usage_get(sigar.sigar.Handle,
                                                     dirname, ptr);

            if (status != Sigar.OK) {
                Marshal.FreeHGlobal(ptr);
                throw Sigar.FindException(sigar, status);
            }

            FileSystemUsage fsusage =
                (FileSystemUsage)Marshal.PtrToStructure(ptr, type);

            Marshal.FreeHGlobal(ptr);

            return fsusage;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct NetInterfaceList {
        private readonly uint number;
        private readonly uint size;
        private readonly IntPtr data;

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_net_interface_list_get(IntPtr sigar, IntPtr iflist);

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_net_interface_list_destroy(IntPtr sigar, IntPtr iflist);

        internal static String[] NativeGet(Sigar sigar) {
            Type type = typeof(NetInterfaceList);
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(type));

            int status =
                sigar_net_interface_list_get(sigar.sigar.Handle, ptr);

            if (status != Sigar.OK) {
                Marshal.FreeHGlobal(ptr);
                throw Sigar.FindException(sigar, status);
            }

            NetInterfaceList ifPtr =
                (NetInterfaceList)Marshal.PtrToStructure(ptr, type);

            String[] iflist = new String[ifPtr.number];

            IntPtr iptr = ifPtr.data;

            for (int i=0; i<ifPtr.number; i++) {
                IntPtr str = Marshal.ReadIntPtr(iptr, i * IntPtr.Size);
                iflist[i] = Marshal.PtrToStringAnsi(str);
            }

            sigar_net_interface_list_destroy(sigar.sigar.Handle, ptr);

            Marshal.FreeHGlobal(ptr);

            return iflist;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct NetAddress {
        internal readonly uint family;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst=4)]
        internal readonly uint[] addr;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct NetInterfaceConfig {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=16)]
        public readonly string Name; //char[16]
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=64)]
        public readonly string Type; //char[64]
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst=256)]
        public readonly string Description; //char[256]
        private readonly NetAddress hwaddr;
        private readonly NetAddress address;
        private readonly NetAddress destination;
        private readonly NetAddress broadcast;
        private readonly NetAddress netmask;
        public readonly long Flags;
        public readonly long Mtu;
        public readonly long Metric;

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_net_interface_config_get(IntPtr sigar, string name,
                                       IntPtr ifconfig);

        internal static NetInterfaceConfig NativeGet(Sigar sigar,
                                                     string name) {
            Type type = typeof(NetInterfaceConfig);
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(type));

            int status = sigar_net_interface_config_get(sigar.sigar.Handle,
                                                        name, ptr);

            if (status != Sigar.OK) {
                Marshal.FreeHGlobal(ptr);
                throw Sigar.FindException(sigar, status);
            }

            NetInterfaceConfig ifconfig =
                (NetInterfaceConfig)Marshal.PtrToStructure(ptr, type);

            Marshal.FreeHGlobal(ptr);

            return ifconfig;
        }

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_net_address_to_string(IntPtr sigar,
                                    IntPtr address,
                                    StringBuilder addr_str);

        private string inet_ntoa(NetAddress address) {
            //XXX seems a little stilly, we can't use &address
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(address));
            StringBuilder buffer = new StringBuilder();
            buffer.Capacity = 46;
            Marshal.StructureToPtr(address, ptr, true);
            sigar_net_address_to_string(IntPtr.Zero, ptr, buffer);
            return buffer.ToString();
        }

        public string Hwaddr {
            get {
                return inet_ntoa(this.hwaddr);
            }
        }

        public string Address {
            get {
                return inet_ntoa(this.address);
            }
        }

        public string Destination {
            get {
                return inet_ntoa(this.destination);
            }
        }

        public string Broadcast {
            get {
                return inet_ntoa(this.broadcast);
            }
        }

        public string Netmask {
            get {
                return inet_ntoa(this.netmask);
            }
        }

        public String FlagsString() {
            long flags = this.Flags;
            String retval = "";

            if (flags == 0)
                retval += "[NO FLAGS] ";
            if ((flags & Sigar.IFF_UP) > 0)
                retval += "UP ";
            if ((flags & Sigar.IFF_BROADCAST) > 0)
                retval += "BROADCAST ";
            if ((flags & Sigar.IFF_DEBUG) > 0)
                retval += "DEBUG ";
            if ((flags & Sigar.IFF_LOOPBACK) > 0)
                retval += "LOOPBACK ";
            if ((flags & Sigar.IFF_POINTOPOINT) > 0)
                retval += "POINTOPOINT ";
            if ((flags & Sigar.IFF_NOTRAILERS) > 0)
                retval += "NOTRAILERS ";
            if ((flags & Sigar.IFF_RUNNING) > 0)
                retval += "RUNNING ";
            if ((flags & Sigar.IFF_NOARP) > 0)
                retval += "NOARP ";
            if ((flags & Sigar.IFF_PROMISC) > 0)
                retval += "PROMISC ";
            if ((flags & Sigar.IFF_ALLMULTI) > 0)
                retval += "ALLMULTI ";
            if ((flags & Sigar.IFF_MULTICAST) > 0)
                retval += "MULTICAST ";
            
            return retval;
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct NetInterfaceStat {
        public readonly long RxPackets;
        public readonly long RxBytes;
        public readonly long RxErrors;
        public readonly long RxDropped;
        public readonly long RxOverruns;
        public readonly long RxFrame;
        public readonly long TxPackets;
        public readonly long TxBytes;
        public readonly long TxErrors;
        public readonly long TxDropped;
        public readonly long TxOverruns;
        public readonly long TxCollisions;
        public readonly long TxCarrier;

        [DllImport(Sigar.LIBSIGAR)]
        private static extern int
        sigar_net_interface_stat_get(IntPtr sigar, string name,
                                     IntPtr ifstat);

        internal static NetInterfaceStat NativeGet(Sigar sigar,
                                                   string name) {
            Type type = typeof(NetInterfaceStat);
            IntPtr ptr = Marshal.AllocHGlobal(Marshal.SizeOf(type));

            int status = sigar_net_interface_stat_get(sigar.sigar.Handle,
                                                      name, ptr);

            if (status != Sigar.OK) {
                Marshal.FreeHGlobal(ptr);
                throw Sigar.FindException(sigar, status);
            }

            NetInterfaceStat ifstat =
                (NetInterfaceStat)Marshal.PtrToStructure(ptr, type);

            Marshal.FreeHGlobal(ptr);

            return ifstat;
        }
    }
}
