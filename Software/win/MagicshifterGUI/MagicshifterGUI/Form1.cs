using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO.Ports;
using System.Diagnostics;
using System.Management;
using System.IO;

namespace MagicshifterGUI
{
    public partial class Form1 : Form
    {
        private String m_fileName;

        public Form1()
        {
            InitializeComponent();

            var port = detectComPort();
            if (port == null) port = "Please connect Magicshifter to usb port."; 
            textBoxPort.Text = port;
            m_fileName = Properties.Settings.Default.DefaultPath;

            labelFileName.Text = m_fileName;


    //        using (var searcher = new ManagementObjectSearcher
    //("SELECT * FROM WIN32_SerialPort"))
    //        {
    //            string[] portnames = SerialPort.GetPortNames();
    //            var ports = searcher.Get().Cast<ManagementBaseObject>().ToList();
    //            var tList = (from n in portnames
    //                         join p in ports on n equals p["DeviceID"].ToString()
    //                         select p["Caption"]).ToList();

    //            foreach (string s in tList)
    //            {
    //                Console.WriteLine(s);
    //            }
    //        }



            //// Get a list of serial port names.
            //string[] ports = SerialPort.GetPortNames();

            //Console.WriteLine("The following serial ports were found:");

            //// Display each port name to the console.
            //foreach (string port in ports)
            //{
            //    Console.WriteLine(port);
            //}

            //Console.ReadLine();
        }

        private string detectComPort()
        {
            using (var searcher = new ManagementObjectSearcher
    ("SELECT * FROM WIN32_SerialPort"))
            {
                string[] portnames = SerialPort.GetPortNames();
                var ports = searcher.Get().Cast<ManagementBaseObject>().ToList();
                var tList = (from n in portnames
                             join p in ports on n equals p["DeviceID"].ToString()
                             select p["Caption"]).ToList();

                int portIndex = 0;
                foreach (string s in tList)
                {
                    if (s.ToLower().Contains("leonardo"))
                    {
                        return portnames[portIndex];
                    }
                    portIndex++;
                }
            }
            return null;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            string path = Properties.Settings.Default.DefaultPort;

            var sector = int.Parse(textBoxSector.Text);
            var port = textBoxPort.Text;
            
            uploadFile(port, m_fileName, sector);

            Properties.Settings.Default.DefaultPort = port;
            Properties.Settings.Default.Save();

        }

        private void uploadFile(string port, string fileName, int sector)
        {
            string workingDirectory = "UploaderBinaries";
            string command = @"UploaderBinaries\MagicTest.exe";
            string arguments = string.Format("up \"{0}\" {1} {2}", fileName, sector, port);

            // Start the child process.
            ProcessStartInfo startInfo = new ProcessStartInfo(command);
            // Redirect the output stream of the child process.
            startInfo.UseShellExecute = false;
            startInfo.RedirectStandardOutput = true;
            startInfo.RedirectStandardError = true;
            startInfo.Arguments = arguments;
            startInfo.WorkingDirectory = workingDirectory;

            var p = Process.Start(startInfo);

            // Do not wait for the child process to exit before
            // reading to the end of its redirected stream.
            // p.WaitForExit();
            // Read the output stream first and then wait.
            string output = p.StandardOutput.ReadToEnd();
            string error = p.StandardError.ReadToEnd();
            textBoxMessage.Text = "arguments: " + Environment.NewLine + arguments + Environment.NewLine + Environment.NewLine +
                "output: " + Environment.NewLine + output + Environment.NewLine + Environment.NewLine +
                "Error: " + error;
            p.WaitForExit();
        }

        private void buttonChooseFile_Click(object sender, EventArgs e)
        {
            OpenFileDialog openFileDialog1 = new OpenFileDialog();

            string path =  Properties.Settings.Default.DefaultPath;
            if (!System.IO.File.Exists(path))
                path = "c:\\";
            openFileDialog1.InitialDirectory = path;
            openFileDialog1.Filter = "txt files (*.txt)|*.txt|All files (*.*)|*.*";
            openFileDialog1.FilterIndex = 2;
            openFileDialog1.RestoreDirectory = true;

            if (openFileDialog1.ShowDialog() == DialogResult.OK)
            {
                m_fileName = openFileDialog1.FileName;

                string extension = Path.GetExtension(m_fileName).ToLowerInvariant();

                Properties.Settings.Default.DefaultPath = m_fileName;
                Properties.Settings.Default.Save();

                

                if (extension == ".png")
                {
                    var image = Image.FromFile(m_fileName);
                    pictureBox1.Image = image;

                    using (Bitmap bmp = new Bitmap(image))
                    {
                      
                    }
                }
                else
                {
                    labelFileName.Text = m_fileName;
                }
            }
        }
    }
}
