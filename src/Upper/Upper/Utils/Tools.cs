using System;
using System.Collections.Generic;
using System.Management;

namespace Upper.Utils
{
    internal class Tools
    {
        /// <summary>
        /// Windows消息编号
        /// </summary>
        public const int WM_DEVICECHANGE = 0x219;
        public const int DBT_DEVICEARRIVAL = 0x8000;
        public const int DBT_DEVICEREMOVECOMPLETE = 0x8004;

        /// <summary>
        /// 终端字符颜色
        /// </summary>
        public enum MyColor
        {
            TX_DEF = 0,
            TX_BLACK = 30,
            TX_RED = 31,
            TX_GREEN = 32,
            TX_YELLOW = 33,
            TX_BLUE = 34,
            TX_WHITE = 37,
            BK_DEF = 0,
            BK_BLACK = 40,
            BK_RED = 41,
            BK_GREEN = 42,
            BK_YELLOW = 43,
            BK_BLUE = 44,
            BK_WHITE = 47,
        };

        /// <summary>
        /// CRC校验
        /// </summary>
        /// <param name="Buf">校验的字节数组</param>
        /// <param name="start">校验的起始位置</param>
        /// <param name="CRC_CNT">校验的数组长度</param>
        /// <returns>该字节数组的奇偶校验字节</returns>
        public static ushort CRC_CHECK(byte[] Buf, int start, int CRC_CNT)
        {
            ushort CRC_Temp = 0xffff;

            for (int i = 0; i < CRC_CNT; i++)
            {
                CRC_Temp ^= Buf[start + i];
                for (int j = 0; j < 8; j++)
                {
                    if (Convert.ToBoolean(CRC_Temp & 0x0001))
                        CRC_Temp = (ushort)((CRC_Temp >> 1) ^ 0xa001);
                    else
                        CRC_Temp = (ushort)(CRC_Temp >> 1);
                }
            }
            return CRC_Temp;
        }

        /// <summary>
        /// 报告指定的 System.Byte[] 在此实例中的第一个匹配项的索引。
        /// </summary>
        /// <param name="srcBytes">被执行查找的 System.Byte[]。</param>
        /// <param name="searchBytes">要查找的 System.Byte[]。</param>
        /// <returns>如果找到该字节数组，则为 searchBytes 的索引位置；如果未找到该字节数组，则为 -1。如果 searchBytes 为 null 或者长度为0，则返回值为 -1。</returns>
        public static int BytesIndexOf(byte[] srcBytes, byte[] searchBytes)
        {
            if (srcBytes == null) { return -1; }
            if (searchBytes == null) { return -1; }
            if (srcBytes.Length == 0) { return -1; }
            if (searchBytes.Length == 0) { return -1; }
            if (srcBytes.Length < searchBytes.Length) { return -1; }

            for (int i = 0; i < srcBytes.Length - searchBytes.Length; i++)
            {
                if (srcBytes[i] == searchBytes[0])
                {
                    if (searchBytes.Length == 1) { return i; }
                    bool flag = true;
                    for (int j = 1; j < searchBytes.Length; j++)
                    {
                        if (srcBytes[i + j] != searchBytes[j])
                        {
                            flag = false;
                            break;
                        }
                    }
                    if (flag) { return i; }
                }
            }
            return -1;
        }

        /// <summary>
        /// 读值线程
        /// </summary>
        private static System.Threading.Thread threadReadValue = null;

        /// <summary>
        /// 局部变量 用来存储返回结果
        /// </summary>
        private static string[] PortNameArray = null;

        /// <summary>
        /// 是否结束
        /// </summary>
        private static bool ReadOver = true;

        /// <summary>
        /// 通过线程获取串口列表
        /// </summary>
        public static string[] GetSerialPortArray()
        {
            PortNameArray = null;
            try
            {
                threadReadValue = new System.Threading.Thread(OnGetSerialPortList)
                {
                    IsBackground = true
                };
                ReadOver = false;
                threadReadValue.Start();

                while (ReadOver == false)
                {
                    System.Threading.Thread.Sleep(200);
                }
                threadReadValue = null;
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("\r\n" + ex.Message);
                Console.WriteLine(ex.StackTrace + "\r\n");
                Console.ForegroundColor = ConsoleColor.White;
            }
            return PortNameArray;
        }

        /// <summary>
        /// 获取串口列表线程
        /// </summary>
        private static void OnGetSerialPortList()
        {
            try
            {
                PortNameArray = MulGetHardwareInfo();
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("\r\n" + ex.Message);
                Console.WriteLine(ex.StackTrace + "\r\n");
                Console.ForegroundColor = ConsoleColor.White;
            }
            ReadOver = true;
        }
        
        /// <summary>
        /// 获取硬件设备信息
        /// </summary>
        /// <returns></returns>
        private static string[] MulGetHardwareInfo()
        {
            List<string> strs = new List<string>();

            try
            {
                using (ManagementObjectSearcher searcher = new ManagementObjectSearcher("SELECT * FROM Win32_PnPEntity"))
                {
                    ManagementObjectCollection hardInfos = searcher.Get();

                    foreach (var hardInfo in hardInfos)
                    {
                        if ((hardInfo.Properties["Name"].Value != null) && hardInfo.Properties["Name"].Value.ToString().Contains("(COM"))
                        {
                            string PortName = hardInfo.Properties["Name"].Value.ToString();
                            int indexOf = PortName.IndexOf("(COM", 0, PortName.Length);

                            string Port = PortName.Substring(indexOf + 1, PortName.Length - indexOf - 2);
                            PortName = PortName.Substring(0, indexOf);

                            strs.Add("[" + Port + "]\t" + PortName);
                        }
                    }
                    searcher.Dispose();
                }
                return strs.ToArray();
            }
            catch (Exception ex)
            {
                Console.ForegroundColor = ConsoleColor.Red;
                Console.WriteLine("\r\n" + ex.Message);
                Console.WriteLine(ex.StackTrace + "\r\n");
                Console.ForegroundColor = ConsoleColor.White;

                return null;
            }
        }
    }
}
