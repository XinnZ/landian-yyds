using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace Upper
{
    /// <summary>
    /// App.xaml 的交互逻辑
    /// </summary>
    public partial class App : Application
    {
        public static readonly string Directory = Environment.CurrentDirectory + '\\';

        private void Application_DispatcherUnhandledException(object sender, System.Windows.Threading.DispatcherUnhandledExceptionEventArgs e)
        {
            // MessageBox.Show(e.Exception.Message, "Oops!", MessageBoxButton.OK, MessageBoxImage.Error);

            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine("\r\n" + e.Exception.Message);
            Console.WriteLine(e.Exception.StackTrace + "\r\n");
            Console.ForegroundColor = ConsoleColor.White;

            e.Handled = true;
        }
    }
}
