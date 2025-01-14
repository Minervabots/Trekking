﻿using Monitor.Classes;
using System;
using System.Windows.Forms;

namespace Monitor
{
    public static class Program
    {
        public static Configuration Configuration { get; set; } = new Configuration();
        /// <summary>
        /// Ponto de entrada principal para o aplicativo.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            Application.Run(new Main());
        }
    }
}
