using MahApps.Metro.Controls;
using PInvoke;
using System;
using System.Collections.ObjectModel;
using System.IO;
using System.IO.Ports;
using System.Text;
using System.Threading;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using Upper.Utils;

namespace Upper
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class WinMain : MetroWindow
    {
        // 串口
        private SerialPort comPort = new SerialPort();
        private bool comIsOpen = false;
        private string comPortName;
        ObservableCollection<string> comList = new ObservableCollection<string>();

        // 串口数据接收 / 发送缓存
        private byte[] comBufferReceive;
        private byte[] comBufferSend;

        // 模式
        private bool mode_Data = false;
        private bool mode_Pid = false;
        private bool mode_Manual = false;

        // 下位机数据缓存
        private byte[] dataContent = new byte[15];
        private int dataIndex = 0;
        private bool dataFull = false;
        
        // 手动控制
        private int manualSpeed = 0;
        private int manualAngle = 0;
        private bool manualLock = false;
        private bool pidLock = false;

        // 线程
        private static Thread threadManual;
        private static Thread threadAnalysis;
        private static Thread threadUpdates;
        private static Thread threadTerminal;

        // 插入设备钩子
        private HwndSource hwndSource;
        private HwndSourceHook hwndSourceHook;

        // 来自下位机的数据
        DateTime dataTimeStart, dataTimeNow = new DateTime();
        long dataTimeStamp = 0;
        public short dataEncoder = 0;
        public short dataMotorPwm = 0;
        public short dataServoPwm = 0;
        public short dataTargetSpeed = 0;
        public short dataTargetAngle = 0;
        public double dataSpeed = 0;
        public double dataRoute = 0;

        public WinMain()
        {
            InitializeComponent();

            Console.Title = "Upper";
        }

        // ASCII 颜色代码
        void ForegroundColor(Tools.MyColor clr)
        {
            Console.WriteLine("\x1b[" + Convert.ToString(((int)clr)) + "m");
        }

        // catch 到 error 输出 ( debug 编译有效 )
        private void Error_Report(Exception ex)
        {
#if DEBUG
            ForegroundColor(Tools.MyColor.TX_RED);
            Console.WriteLine("\r\n" + ex.Message);
            Console.WriteLine(ex.StackTrace + "\r\n");
            ForegroundColor(Tools.MyColor.TX_WHITE);
#endif
        }

        // 控件 禁用/使能
        private void Control_Enable(bool Enable)
        {
            Button[] controls = { Btn_Manual, Btn_RunPID, Btn_RunOpen, Btn_RunClose, Btn_RunUpload, Btn_RunUploadClear };
            foreach (Button control in controls)
            {
                control.IsEnabled = Enable;
                control.ToolTip = control.Content;
            }
        }

        // 打开/关闭串口
        private void Button_ComSwitch_Click(object sender, RoutedEventArgs e)
        {
            if (comIsOpen)
                ComPortClose();
            else
                ComPortOpen();
        }

        // 打开/关闭手动控制
        private void Button_Manual_Click(object sender, RoutedEventArgs e)
        {
            mode_Pid = false;

            if (Cmd_Manual.IsEnabled)
            {
                Cmd_Manual.IsEnabled = false;
                mode_Manual = false;

                Thread.Sleep(50);
                threadManual = null;

                byte[] cmd = { 0xF1, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x01, 0xF2 };
                ComPortSend(cmd);
            }
            else
            {
                Cmd_Manual.IsEnabled = true;
                mode_Manual = true;

                threadManual = new Thread(Manual_Task) { IsBackground = true };
                threadManual.Start();

                ComPortSend("run --manual\n");
            }
        }

        // 打开车模自动运行
        private void Button_RunOpen_Click(object sender, RoutedEventArgs e)
        {
            ComPortSend("run --auto\n");
        }

        // 关闭车模所有控制
        private void Button_RunClose_Click(object sender, RoutedEventArgs e)
        {
            mode_Data = false;
            mode_Pid = false;
            mode_Manual = false;
            ComPortSend("run --stop\n");
            comPort.DiscardInBuffer(); // 清接收缓存
        }

        // 清除从下位机接收的数据
        private void Button_RunUploadClear_Click(object sender, RoutedEventArgs e)
        {
            dataEncoder = 0;
            dataMotorPwm = 0;
            dataServoPwm = 0;
            dataTargetSpeed = 0;
            dataTargetAngle = 0;
            dataSpeed = 0;
            dataRoute = 0;
            Web.InvokeScript("DeleteDatas");
        }

        // 打开/关闭下位机数据传输
        private void Button_RunUpload_Click(object sender, RoutedEventArgs e)
        {
            if (mode_Data)
            {
                mode_Data = false;
                mode_Pid = false;
                mode_Manual = false;
                ComPortSend("run --stop\n");
                comPort.DiscardInBuffer(); // 清接收缓存
            }
            else
            {
                ComPortSend("run --upload\n");
                mode_Data = true;
                dataTimeStart = DateTime.Now;
            }
        }

        // 打开/关闭PID模式
        private void Button_RunPID_Click(object sender, RoutedEventArgs e)
        {
            if (mode_Pid)
            {
                mode_Pid = false;
                Cmd_Manual.IsEnabled = false;
            }
            else
            {
                mode_Pid = true;
                Cmd_Manual.IsEnabled = true;
            }
        }

        // 修改下位机PID参数
        private void Button_PID_Click(object sender, RoutedEventArgs e)
        {
            try
            {
                Button Btn = (Button)sender;

                if ("PID_S" == Btn.Name.ToString())
                {
                    string cmd = string.Format(
                        "pid -s -f {0} {1} {2}\n", 
                        Convert.ToDouble(PID_S_P.Text),
                        Convert.ToDouble(PID_S_I.Text),
                        Convert.ToDouble(PID_S_D.Text)
                    );
                    ComPortSend(cmd);
                }
                else if ("PID_A" == Btn.Name.ToString())
                {
                    string cmd = string.Format(
                        "pid -a -f {0} {1} {2}\n", 
                        Convert.ToDouble(PID_A_P.Text),
                        Convert.ToDouble(PID_A_I.Text),
                        Convert.ToDouble(PID_A_D.Text)
                    );
                    ComPortSend(cmd);
                }
                else if ("PID_O" == Btn.Name.ToString())
                {
                    string cmd = string.Format(
                        "pid -o -f {0} {1} {2}\n",
                        Convert.ToDouble(PID_O_P.Text),
                        Convert.ToDouble(PID_O_I.Text),
                        Convert.ToDouble(PID_O_D.Text)
                    );
                    ComPortSend(cmd);
                }
            }
            catch (Exception ex)
            {
                Error_Report(ex);
            }
        }
        
        // 打开Git仓库
        private void Button_GitLink_Click(object sender, RoutedEventArgs e)
        {
        }

        // 手动控制 指令发送
        private void Manual_Task()
        {
            byte[] cmd = { 0xF1, 0xE1, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF2 };

            while (mode_Manual)
            {
                cmd[2] = (byte)((manualSpeed >> 8) & 0xFF);
                cmd[3] = (byte)((manualSpeed >> 0) & 0xFF);

                cmd[4] = (byte)((manualAngle >> 8) & 0xFF);
                cmd[5] = (byte)((manualAngle >> 0) & 0xFF);

                ComPortSend(cmd);
                Thread.Sleep(50);
            }
        }

        // 手动控制 按下
        private void Manual_KeyDown(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.A: manualAngle = 60; break;
                case Key.D: manualAngle = -60; break;

                case Key.W: manualSpeed = 4000; break;
                case Key.S: manualSpeed = -4000; break;
                case Key.Z: manualSpeed =  500; break;
                case Key.X: manualSpeed = 1000; break;
                case Key.C: manualSpeed = 1500; break;
                case Key.V: manualSpeed = 2000; break;
                case Key.B: manualSpeed = 3000; break;
                case Key.N: manualSpeed = 4000; break;
                case Key.M: manualSpeed = 5000; break;
            }

            if (mode_Pid && pidLock == false)
            {
                pidLock = true;
                ComPortSend(String.Format("run -s {0}\n", manualSpeed));
            }

            if (e.KeyboardDevice.Modifiers == ModifierKeys.Shift && manualLock == false)
            {
                manualLock = true;
                manualSpeed = Convert.ToInt32(manualSpeed * 1.5f);
                manualAngle = Convert.ToInt32(manualAngle * 1.5f);
            }

            Cmd_Manual.Text = e.Key.ToString();
        }

        // 手动控制 抬起
        private void Manual_KeyUp(object sender, KeyEventArgs e)
        {
            switch (e.Key)
            {
                case Key.Z:
                case Key.X:
                case Key.C:
                case Key.V:
                case Key.B:
                case Key.N:
                case Key.M:
                case Key.W:
                case Key.S: 
                    manualSpeed = 0;    break;
                case Key.A:
                case Key.D: 
                    manualAngle = 0;    break;
                case Key.LeftShift: 
                    manualLock = false; break;
            }

            if (mode_Pid && pidLock == true)
            {
                pidLock = false;
                ComPortSend(String.Format("run -s 0\n"));
            }

            Cmd_Manual.Text = String.Empty;
        }

        // 加载事件
        private void MetroWindow_Loaded(object sender, RoutedEventArgs e)
        {
            // 禁用界面按键防止未连接时按下
            Control_Enable(false);

            // 串口号数据绑定
            WM_ComPortName.ItemsSource = comList;

            // 数据解析线程
            threadAnalysis = new Thread(Data_Analysis) { IsBackground = true };
            threadAnalysis.Start();

            // 数据更新线程
            threadUpdates = new Thread(Data_Updates) { IsBackground = true };
            threadUpdates.Start();

            // 终端线程
            threadTerminal = new Thread(Terminal_Task) { IsBackground = true };
            threadTerminal.Start();

            // 监听 Windows 消息
            hwndSource = PresentationSource.FromVisual(this) as HwndSource;
            hwndSourceHook = new HwndSourceHook(DeviceChanged);
            if (hwndSource != null && hwndSourceHook != null)
                hwndSource.AddHook(hwndSourceHook);

            ComPortNameRefresh();

            WebLoader();
        }

        // 退出事件
        private void MetroWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            ComPortClose();

            if (hwndSource != null && hwndSourceHook != null)
                hwndSource.RemoveHook(hwndSourceHook);

            Application.Current.Shutdown(); // 先停止线程, 然后终止进程
            Environment.Exit(0); // 直接终止进程
        }

        // echarts 内容加载
        private void WebLoader()
        {
            string path = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData) + "\\Upper";
            if (Directory.Exists(path))
                Directory.Delete(path, true);
            Directory.CreateDirectory(path);

            Uri uHtml = new Uri("/ECharts/index.html", UriKind.Relative);
            Uri uJs = new Uri("/ECharts/echarts.min.js", UriKind.Relative);

            StreamReader srHtml = new StreamReader(Application.GetResourceStream(uHtml).Stream);
            StreamReader srJs = new StreamReader(Application.GetResourceStream(uJs).Stream);

            Files.write(path + "\\index.html", srHtml.ReadToEnd(), false);
            Files.write(path + "\\echarts.min.js", srJs.ReadToEnd(), false);

            srHtml.Dispose();
            srJs.Dispose();

            Web.Source = new Uri(path + "\\index.html");
        }

        // 打开串口
        private bool ComPortOpen()
        {
            try
            {
                comPortName = WM_ComPortName.Text;
                string portName = comPortName;
                int index1 = portName.IndexOf("[COM", 0, portName.Length);
                int index2 = portName.IndexOf("]", index1, portName.Length);
                string Port = portName.Substring(index1 + 1, index2 - 1);

                comPort.PortName = Port;
                comPort.BaudRate = Convert.ToInt32(WM_ComBaudrate.Text);
                comPort.DataBits = 8;
                comPort.Parity = Parity.None;
                comPort.StopBits = StopBits.One;
                comPort.ReadBufferSize = 1024;
                comPort.WriteBufferSize = 1024;
                comPort.ReceivedBytesThreshold = 1;

                comPort.Open();

                comPort.DiscardInBuffer();
                comPort.DiscardOutBuffer();

                comIsOpen = true;
                Control_Enable(true);

                WM_ComPortName.IsEnabled = false;
                WM_ComBaudrate.IsEnabled = false;
                WM_ComSwitch.Content = "关闭串口";

                return true;
            }
            catch (Exception ex)
            {
                Error_Report(ex);

                comIsOpen = false;
                WM_ComPortName.IsEnabled = true;
                WM_ComBaudrate.IsEnabled = true;
                WM_ComSwitch.Content = "打开串口";

                return false;
            }
        }

        // 关闭串口
        private void ComPortClose()
        {
            try
            {
                if (mode_Manual)
                {
                    mode_Manual = false;
                    Cmd_Manual.IsEnabled = false;
                    Thread.Sleep(50);
                    threadManual = null;
                }
                mode_Data = false;
                mode_Pid = false;

                comPortName = "";
                comIsOpen = false;

                Control_Enable(false);

                WM_ComPortName.IsEnabled = true;
                WM_ComBaudrate.IsEnabled = true;
                WM_ComSwitch.Content = "打开串口";

                comPort.DiscardOutBuffer(); // 清发送缓存
                comPort.DiscardInBuffer(); // 清接收缓存
                comPort.Close();
            }
            catch (Exception ex)
            {
                 Error_Report(ex);
            }
        }

        // 串口号刷新
        private void ComPortNameRefresh()
        {
            try
            {
                comList.Clear();
                foreach (var str in Tools.GetSerialPortArray())
                    comList.Add(str);

                if (comList.Count > 0)
                    WM_ComPortName.SelectedItem = comList[0];
            }
            catch (Exception ex)
            {
                Error_Report(ex);
            }
        }

        // 串口发送数据
        private void ComPortSend(string message)
        {
            try
            {
                var send = Encoding.Default.GetBytes(message.ToCharArray());
                comPort.Write(send, 0, send.Length);
            }
            catch (Exception ex)
            {
                Error_Report(ex);
            }
        }
        private void ComPortSend(byte[] bytes)
        {
            try
            {
                comPort.Write(bytes, 0, bytes.Length);
            }
            catch (Exception ex)
            {
                Error_Report(ex);
            }
        }


        // 串口接收数据解析
        private void Data_Analysis()
        {
            while (true)
            {
                // 串口未打开
                while (false == comIsOpen)
                    Thread.Sleep(10);

                // 接收手动输入的指令并显示
                if (false == mode_Data)
                {
                    try
                    {
                        comBufferReceive = new byte[comPort.BytesToRead];
                        comPort.Read(comBufferReceive, 0, comBufferReceive.Length);
                        Console.Write(Encoding.Default.GetString(comBufferReceive));
                    }
                    finally { Thread.Sleep(1); }
                    continue;
                }

                // 接收下位机上传的数据
                if (true == mode_Data)
                {
                    try
                    {
                        comPort.Read(dataContent, dataIndex++, 1);

                        if (0xF1 != dataContent[0])
                        { dataIndex = 0; continue; }
                        if (0xE2 != dataContent[1] && dataIndex >= 2)
                        { dataIndex = 0; continue; }
                        if (15 <= dataIndex)
                        {
                            if (0xF2 == dataContent[(15 - 1)])
                            {
                                dataTimeStamp = (long)(DateTime.Now.ToUniversalTime() - dataTimeStart.ToUniversalTime()).TotalMilliseconds;
                                
                                dataEncoder = (short)(dataContent[2] << 8 | dataContent[3] & 0xff);
                                dataMotorPwm = (short)(dataContent[4] << 8 | dataContent[5] & 0xff);
                                dataServoPwm = (short)(dataContent[6] << 8 | dataContent[7] & 0xff);
                                dataTargetSpeed = (short)(dataContent[8] << 8 | dataContent[9] & 0xff);
                                dataTargetAngle = (short)(dataContent[10] << 8 | dataContent[11] & 0xff);

                                /* 1 pulse = (πD / Line) * (Master gear / slave gear) / time (m/s) */
                                /* 1 pulse = (π*0.0625 / 5000) * (14 / 38) / 0.01 (m/s) */
                                //Run_Params.CtSpeed = Run_Params.CtEncoder_new * 0.0014815f;
                                dataSpeed = dataEncoder * 0.0014815f;

                                // s(cm) = v(m/s) * t(ms) 
                                dataRoute += dataSpeed * 2.0f;

                                dataFull = true;
                            }
                            dataIndex = 0;
                            dataContent[0] = 0x00;
                        }
                    }
                    catch { }
                }
            }
        }

        private void Data_Updates()
        {
            while (true)
            {
                // 串口未打开
                while (false == comIsOpen)
                    Thread.Sleep(10);

                if (dataFull)
                {
                    dataFull = false;

                    Dispatcher.Invoke(new Action(() =>
                    {
                        Data_TargetSpeed.Text = Convert.ToString(dataTargetSpeed);
                        Data_TargetAngle.Text = Convert.ToString(dataTargetAngle);

                        Data_Encoder.Text = Convert.ToString(dataEncoder);
                        Data_Speed.Text = Convert.ToString(Math.Round(dataSpeed, 3));
                        Data_Route.Text = Convert.ToString(Math.Round(dataRoute, 3));

                        Data_MotorPWM.Text = Convert.ToString(dataMotorPwm);
                        Data_ServoPWM.Text = Convert.ToString(dataServoPwm);
                    }));

                    Dispatcher.Invoke(new Action(() =>
                    {
                        DateTime sTime = dataTimeNow.AddMilliseconds(dataTimeStamp).ToLocalTime();
                        Web.InvokeScript(
                            "PushDatas",
                            string.Format("{0:D3}:{1:D3}", sTime.Second + 60 * sTime.Minute, sTime.Millisecond),
                            dataEncoder,
                            dataTargetSpeed
                            );
                    }), System.Windows.Threading.DispatcherPriority.ContextIdle);

                    /*
                    Files.write(App.Directory + "datas.txt", string.Join(",", Array.ConvertAll(dataContent, Convert.ToString)), true);
                    Files.write(App.Directory + "datas.txt",
                                dataEncoder.ToString() + "," +
                                dataMotorPwm.ToString() + "," +
                                dataServoPwm.ToString() + "," +
                                dataTargetSpeed.ToString() + "," +
                                dataTargetAngle.ToString(), true);
                    Files.write(App.Directory + "datas.txt", "\n", true);
                    */
                }
                else { Thread.Sleep(1); }
            }
        }

        // 设备检测
        private IntPtr DeviceChanged(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
        {
            if (msg == Tools.WM_DEVICECHANGE)
            {
                ComPortNameRefresh();
                switch (wParam.ToInt32())
                {
                    case Tools.DBT_DEVICEARRIVAL:
                        if (comIsOpen)
                            WM_ComPortName.SelectedValue = comPortName;
                        break;

                    case Tools.DBT_DEVICEREMOVECOMPLETE:
                        if (comIsOpen)
                        {
                            foreach (var com in comList)
                                if (com == comPortName)
                                    break;

                            ComPortClose();
                        }
                        break;
                }
            }
            return IntPtr.Zero;
        }

        // 代码出处：https://stackoverflow.com/questions/34073467/ansi-coloring-console-output-with-net
        private bool TryEnableAnsiCodesForHandle(Kernel32.StdHandle stdHandle = Kernel32.StdHandle.STD_OUTPUT_HANDLE)
        {
            var consoleHandle = Kernel32.GetStdHandle(stdHandle);
            if (Kernel32.GetConsoleMode(consoleHandle, out var consoleBufferModes) &&
                consoleBufferModes.HasFlag(Kernel32.ConsoleBufferModes.ENABLE_VIRTUAL_TERMINAL_PROCESSING))
                return true;

            consoleBufferModes |= Kernel32.ConsoleBufferModes.ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            return Kernel32.SetConsoleMode(consoleHandle, consoleBufferModes);
        }

        // 终端
        private void Terminal_Task()
        {
            // 启用 ASCII 颜色代码支持
            TryEnableAnsiCodesForHandle();

            Console.Clear();
            ForegroundColor(Tools.MyColor.TX_RED);
            Console.WriteLine("    _/      _/                                  _/   ");
            Console.WriteLine("     _/  _/      _/_/_/  _/_/_/  _/_/      _/_/_/    ");
            Console.WriteLine("      _/      _/        _/    _/    _/  _/    _/     ");
            Console.WriteLine("   _/  _/    _/        _/    _/    _/  _/    _/      ");
            Console.WriteLine("_/      _/    _/_/_/  _/    _/    _/    _/_/_/       ");
            Console.WriteLine("");
            Console.WriteLine("> Teams: Landian YYDS Car");
            Console.WriteLine("> Topic: Baidu Complete Model Group");
            Console.WriteLine("> Build: " + File.GetLastWriteTime(GetType().Assembly.Location));
            ForegroundColor(Tools.MyColor.TX_WHITE);

            // 等待连接
            while (false == comIsOpen)
                Thread.Sleep(10);

            // 开始任务
            while (true)
            {
                // 等待重连
                while (false == comIsOpen)
                {
                    Thread.Sleep(10);
                    Console.Clear();
                }

                // 等待输入
                while (true == comIsOpen)
                {
                    Thread.Sleep(1);

                    var key = Console.ReadKey(true);
                    switch (key.Key)
                    {
                        case ConsoleKey.UpArrow:
                            comBufferSend = Encoding.Default.GetBytes("\x1B[A");
                            break;
                        case ConsoleKey.DownArrow:
                            comBufferSend = Encoding.Default.GetBytes("\x1B[B");
                            break;
                        case ConsoleKey.LeftArrow:
                            comBufferSend = Encoding.Default.GetBytes("\x1B[D");
                            break;
                        case ConsoleKey.RightArrow:
                            comBufferSend = Encoding.Default.GetBytes("\x1B[C");
                            break;
                        default:
                            comBufferSend = Encoding.Default.GetBytes(key.KeyChar.ToString());
                            break;
                    }

                    ComPortSend(comBufferSend);
                }
            }
        }
    }
}
