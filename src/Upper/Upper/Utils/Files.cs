using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Web.Script.Serialization;

namespace Upper.Utils
{
    internal class Files
    {
        public static JavaScriptSerializer serializer = new JavaScriptSerializer();

        // 往文件中写入数据
        public static void write(string fileName, string contents, bool append)
        {
            if (!File.Exists(fileName))
            {
                StreamWriter streamWriter = new StreamWriter(fileName, append);
                streamWriter.WriteLine(contents);
                streamWriter.Dispose();
            }
        }

        // 从文件中读取数据
        public static List<string> read(string fileName)
        {
            List<string> ts = new List<string>();

            if (File.Exists(fileName))
            {
                using (StreamReader sr = new StreamReader(fileName, Encoding.Default))
                {
                    while (sr.Peek() > 0)
                    {
                        ts.Add(sr.ReadLine());
                    }
                }
            }

            return ts;
        }
    }
}
