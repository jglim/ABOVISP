using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.IO.Ports;


namespace abovflash
{
    class Program
    {
        const int FLASH_SIZE = 4096;
        
        // post compilation cleanup
        static void CleanFolder()
        {
            string[] filesToRemove = { "_.hms800elf.x", "_.temp", "main.hex", "main.map", "main.out", "main.lst", "main.o" };
            foreach (string fileToRemove in filesToRemove)
            {
                
                if (File.Exists(fileToRemove))
                {
                    File.Delete(fileToRemove);
                }
            }
        }

        // sanity check before starting
        static bool CheckIfToolchainExists()
        {
            string[] filesToCheck = { @"C:\HMS800C\options\MC81F4204.opt", "main.c", @"C:\HMS800C\bin\hms800-cc.exe", @"C:\HMS800C\bin\hms800-elf-objcopy.exe" };
            foreach (string fileToCheck in filesToCheck)
            {
                if (!File.Exists(fileToCheck))
                {
                    Console.Error.Write("Toolchain Error: {0} could not be found.\n", fileToCheck);
                    Console.Error.Write("Please install the ABOV HMS800 toolchain first!\n");
                    return false;
                }
            }
            return true;
        }

        // used to start the batch file
        static void StartProcessWithOutput(string processFileName, string processArguments)
        {
            System.Diagnostics.Process process = new System.Diagnostics.Process();
            process.StartInfo.FileName = processFileName;
            process.StartInfo.Arguments = processArguments;

            process.EnableRaisingEvents = true;
            process.StartInfo.RedirectStandardInput = true;
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.RedirectStandardOutput = true;
            process.StartInfo.RedirectStandardError = true;
            process.OutputDataReceived += (sender, args) => Console.Write("{0}", args.Data);
            process.Start();
            process.BeginOutputReadLine();

            process.WaitForExit();
        }

        // the HMS800 compiler outputs the firmware in a srec file but only the firmware's 4k bytes are required
        static bool ParseSrec(ref List<byte> mainFlash, ref List<byte> bootFlash, ref List<byte> entireFlashReturn)
        {
            List<string> srecRows = new List<string>(File.ReadAllText("main.hex").Trim().Split(new string[] {"\r\n"}, StringSplitOptions.None));
            byte[] entireFlash = new byte[FLASH_SIZE];
            

            foreach (string srecRow in srecRows)
            {
                if (srecRow.Length % 2 != 0)
                {
                    Console.WriteLine("Invalid SREC: odd character length");
                    return false;
                }

                List<string> rowStringBytes = ConvertSrecRowToStringBytes(srecRow);

                // we only want data records
                if (rowStringBytes[0] == "S1")
                {
                    byte recordLength = byte.Parse(rowStringBytes[1], System.Globalization.NumberStyles.HexNumber);
                    byte providedChecksum = byte.Parse(rowStringBytes[rowStringBytes.Count - 1], System.Globalization.NumberStyles.HexNumber);
                    UInt16 sumOfBytes = 0;
                    for (int i = 1; i < rowStringBytes.Count - 1; i++)
                    {
                        sumOfBytes += byte.Parse(rowStringBytes[i], System.Globalization.NumberStyles.HexNumber);
                    }
                    sumOfBytes = (UInt16)(sumOfBytes & (byte)0xFF);
                    byte checksum = (byte)sumOfBytes;
                    checksum = (byte)~checksum;

                    if (checksum != providedChecksum)
                    {
                        throw new NotImplementedException("Checksum on SREC failed to verify.");
                    }

                    // the 81F4204 flash layout starts from 0xF000 and spans 4k. we'll store it in a byte[4096] 
                    UInt16 address = (UInt16)(byte.Parse(rowStringBytes[2], System.Globalization.NumberStyles.HexNumber) << 8);
                    address |= byte.Parse(rowStringBytes[3], System.Globalization.NumberStyles.HexNumber);

                    UInt16 offsetAddress = (UInt16)(address - 0xF000);

                    for (int i = 4; i < rowStringBytes.Count - 1; i++)
                    {
 
                        entireFlash[offsetAddress + i - 4] = byte.Parse(rowStringBytes[i], System.Globalization.NumberStyles.HexNumber);
                    }
                    

                }
                
            }

            // copy the last 32 bytes into bootFlash.. in case future programmer doesn't load all 4096 bytes
            for (int i = (FLASH_SIZE - 32); i < FLASH_SIZE; i++)
            {
                bootFlash.Add(entireFlash[i]);
            }

            int lastPositionThatWasntAZero = 0;
            for (int i = 0; i < (FLASH_SIZE - 32); i++)
            {
                if (entireFlash[i] != 0)
                {
                    lastPositionThatWasntAZero = i;
                }
            }
            // entireFlash = mainFlash + zeroes + bootFlash
            for (int i = 0; i <= lastPositionThatWasntAZero; i++)
            {
                mainFlash.Add(entireFlash[i]);
            }

            entireFlashReturn = new List<byte>(entireFlash);
            return true;
        }

        // parse srec file's row and return as stringified bytes
        static List<string> ConvertSrecRowToStringBytes(string srecRow)
        {
            char[] srecRowChars = srecRow.ToCharArray();
            List<string> result = new List<string>();
            for (int i = 0; i < srecRowChars.Length; i++)
            {
                if (i % 2 == 0)
                {
                    result.Add(srecRowChars[i].ToString() + srecRowChars[i + 1].ToString());
                }
            }
            return result;
        }

        // start the hms800 compiler
        static void RunHMS800(string workingPath)
        {

            Console.WriteLine("Setting directory to {0}", workingPath);
            Directory.SetCurrentDirectory(workingPath);

            if (!CheckIfToolchainExists())
            {
                Console.ReadLine();
                return;
            }

            CleanFolder();

            File.WriteAllText("build.bat", "@echo off" +
                "\r\n" + "hms800-cc -ramclr -mshort -Os -g -Wa,\"-mhms800\"  --option-file=C:\\HMS800C\\options\\MC81F4204.opt -Wa,\"-ahls=.\\main.lst\" -c -o .\\main.o .\\main.c" +
                "\r\n" + "hms800-cc -ramclr -mshort -Wl,\"-Map=.\\main.map\" --option-file=C:\\HMS800C\\options\\MC81F4204.opt -o .\\main.out .\\main.o" +
                "\r\n" + "hms800-elf-objcopy -O srec .\\main.out .\\main.hex" +
                "\r\n");

            System.Diagnostics.Process p = System.Diagnostics.Process.Start("build.bat");
            p.WaitForExit();

            File.Delete("build.bat");


            StringBuilder firmwareAscii = new StringBuilder();

            if (!File.Exists("main.hex"))
            {
                Console.Error.WriteLine("Build Failed");

                // launch the build again with a pause to cough up the error
                File.WriteAllText("build.bat", "@echo off" +
                "\r\n" + "hms800-cc -ramclr -mshort -Os -g -Wa,\"-mhms800\"  --option-file=C:\\HMS800C\\options\\MC81F4204.opt -Wa,\"-ahls=.\\main.lst\" -c -o .\\main.o .\\main.c" +
                "\r\n" + "hms800-cc -ramclr -mshort -Wl,\"-Map=.\\main.map\" --option-file=C:\\HMS800C\\options\\MC81F4204.opt -o .\\main.out .\\main.o" +
                "\r\n" + "hms800-elf-objcopy -O srec .\\main.out .\\main.hex" +
                "\r\npause");
                System.Diagnostics.Process p2 = System.Diagnostics.Process.Start("build.bat");
                p2.WaitForExit();

                File.Delete("build.bat");

                Environment.Exit(1);
            }
            else
            {

                List<byte> mainFlash = new List<byte>();
                List<byte> bootFlash = new List<byte>();
                List<byte> entireFlash = new List<byte>();

                ParseSrec(ref mainFlash, ref bootFlash, ref entireFlash);


                firmwareAscii.Append("{");
                foreach (byte b in entireFlash)
                {
                    firmwareAscii.AppendFormat("{0:X2}", b);
                }
                firmwareAscii.Append("}");

                System.Windows.Forms.Clipboard.SetText(firmwareAscii.ToString());
                //Console.Error.Write("Compilation Complete. Flash Size: {0} bytes out of 4096 bytes", mainFlash.Count);
                File.WriteAllText("firmware_size.txt", mainFlash.Count.ToString());
                File.WriteAllText("firmware_command.txt", firmwareAscii.ToString());
                Console.Error.Write("Compilation Complete\n");
            }
            
        }

        // sends a firmware down to ABOVISP via serial
        static void UploadViaSerial(string firmwareAscii, string desiredPort)
        {
            // dotnet's serialport doesn't quite work out as expected...
            foreach (string port in SerialPort.GetPortNames())
            {
                if (port == desiredPort)
                {
                    Console.Error.WriteLine("Writing to {0}", port);
                    SerialPort sp = new SerialPort(port, 115200); // atmega32u4's virtual port does not care about baud
                    sp.DataReceived += delegate (object sender, SerialDataReceivedEventArgs e)
                    {
                        // i have no idea if things will break if i call sp as-is from a delegate
                        SerialPort spAgain = (SerialPort)sender;
                        Console.Error.Write(spAgain.ReadExisting());
                    };

                    if (sp.IsOpen == true)
                    {
                        Console.Error.WriteLine("Port {0} is already in use, stopping.", port);
                        Environment.Exit(1);
                    }
                    sp.Open();
                    sp.Write(firmwareAscii);

                    Console.Error.WriteLine("Programming Complete");
                    return;
                }
            }
            Console.Error.WriteLine("Port {0} does not exist", desiredPort);
            Environment.Exit(1);
        }

        // calculate the fuses / options
        static byte CalculateSelectedOptions(string osc, string lvr, string lockbits)
        {
            /* 
            LOCK:
            OFF  00000000 0x00
            ON   00010000 0x08
            
            OSC:
            EXTR 00000000 0x00
            4MHZ 10000000 0x01
            2MHZ 01000000 0x02
            1MHZ 11000000 0x03
            8MHZ 00100000 0x04
            XTAL 11100000 0x07

            LVR:
            OFF  00000001 0x80
            2V4  00000000 0x00
            2V7  00000100 0x20
            3V0  00000010 0x40
            4V0  00000110 0x60
            */
            byte optionValue = 0x00;

            if      (lockbits == "ON" ) { optionValue |= 0x08; }
            else if (lockbits == "OFF") { optionValue |= 0x00; }

            if      (osc == "EXTR") { optionValue |= 0x00; }
            else if (osc == "4MHZ") { optionValue |= 0x01; }
            else if (osc == "2MHZ") { optionValue |= 0x02; }
            else if (osc == "1MHZ") { optionValue |= 0x03; }
            else if (osc == "8MHZ") { optionValue |= 0x04; }
            else if (osc == "XTAL") { optionValue |= 0x07; }

            if      (lvr == "OFF") { optionValue |= 0x80; }
            else if (lvr == "2V4") { optionValue |= 0x00; }
            else if (lvr == "2V7") { optionValue |= 0x20; }
            else if (lvr == "3V0") { optionValue |= 0x40; }
            else if (lvr == "4V0") { optionValue |= 0x60; }

            return optionValue;
        }

        static void StartCompile(string writeDummyCtagsToThisPath)
        {
            // banner
            Console.Error.WriteLine("   _   ___  _____   _____ _      _   ___ _  _ \n  /_\\ | _ )/ _ \\ \\ / / __| |    /_\\ / __| || |\n / _ \\| _ \\ (_) \\ V /| _|| |__ / _ \\\\__ \\ __ |\n/_/ \\_\\___/\\___/ \\_/ |_| |____/_/ \\_\\___/_||_|\n");

            // check if it has already been compiled! the arduino ide calls abovflash several times with similar params
            if (File.Exists(writeDummyCtagsToThisPath))
            {
                // HACK: just let it recompile - it's fast and saves caching headaches :(
                // return;
            }

            // write a blank ctags file because arduino IDE preprocessor needs it to not throw an exception
            File.WriteAllText(writeDummyCtagsToThisPath, "// " + Guid.NewGuid().ToString());

            // get the temporary directory from the ctags path
            List<string> compileTemporaryDirectorySegments = new List<string>(writeDummyCtagsToThisPath.Split('\\'));
            // traverse 2 backslashes back
            compileTemporaryDirectorySegments.RemoveRange(compileTemporaryDirectorySegments.Count - 2, 2);
            string compileTemporaryDirectory = string.Join("\\", compileTemporaryDirectorySegments);
            Console.Error.Write("Temporary Directory: {0}\n", compileTemporaryDirectory);

            // parse build.options.json .. without a json parser
            string buildOptionsFilePath = compileTemporaryDirectory + "\\build.options.json";
            // read and normalize quotes (in case they use singlequotes)
            string buildOptionsJson = System.IO.File.ReadAllText(buildOptionsFilePath).Replace('\'', '"');

            // looking for key "sketchLocation"
            string keyName = "\"sketchLocation\"";
            if (buildOptionsJson.Contains(keyName))
            {
                string sketchLocationValue = buildOptionsJson.Split(new string[] { keyName }, StringSplitOptions.None)[1]
                    .Split(new string[] { "\"" }, StringSplitOptions.None)[1]
                    .Replace("\\\\", "\\");

                // merge headers into the sketch and place it inside a new folder under the temporary directory

                // get abovflash's location
                string codebaseLocation = System.Reflection.Assembly.GetExecutingAssembly().CodeBase.Replace("file:///", "").Replace("/", "\\");
                List<string> assemblyPathSegments = new List<string>(codebaseLocation.Split('\\'));
                assemblyPathSegments.RemoveAt(assemblyPathSegments.Count - 1);
                string assemblyPath = string.Join("\\", assemblyPathSegments);

                // create the working folder
                System.IO.Directory.CreateDirectory(compileTemporaryDirectory + "\\abovflash");

                // merge and write
                string compilationHeaders = System.IO.File.ReadAllText(assemblyPath + "\\main_arduino.c");
                string sketchData = System.IO.File.ReadAllText(sketchLocationValue);
                System.IO.File.WriteAllText(compileTemporaryDirectory + "\\abovflash\\main.c", compilationHeaders + sketchData);

                // start the compile!
                RunHMS800(compileTemporaryDirectory + "\\abovflash");

            }
            else
            {
                Console.Error.WriteLine("Error: {0} key not found in build options. Exiting", keyName);
                Environment.Exit(1);
            }

        }

        [STAThread]
        static void Main(string[] args)
        {
            string uploadPort =             "UNSET";
            string uploadConfigCpuFreq =    "UNSET";
            string uploadConfigLvr =        "UNSET";
            string uploadConfigLockbits =   "UNSET";
            
            foreach (string arg in args)
            {
                // look out for "\preproc\ctags_target_for_gcc_minus_e.cpp" in args
                // this arg is provided once during compile and is a great time to actually get the compiling going
                // if we find it, extract/create all the required paths and files (temp folder, abovflash folder, sketch, header, merged sketch) 
                // then start compiling 
                // this breaks in arduino 1.8.2 wtf
                if (arg.Contains("ctags_target_for_gcc_minus_e"))
                {
                    StartCompile(arg);
                    break;
                }
                
                // arduino IDE requesting for built binary's firmware size
                if (arg.Contains("firmware_size.txt"))
                {
                    if (System.IO.File.Exists(arg))
                    {
                        string fileSize = System.IO.File.ReadAllText(arg);
                        if (fileSize.Length > 4)
                        {
                            Console.WriteLine(".data 0");
                        }
                        else
                        {
                            Console.WriteLine(".data {0}", fileSize);
                        }
                    }
                    else
                    {
                        Console.WriteLine(".data 0");
                    }
                }

                // arduino IDE requesting upload (config)

                if (arg.Contains("PORT_"))
                {
                    uploadPort = arg.Replace("PORT_", "");
                }
                else if (arg.Contains("FREQ_"))
                {
                    uploadConfigCpuFreq = arg.Replace("FREQ_", "");
                }
                else if (arg.Contains("LVR_"))
                {
                    uploadConfigLvr = arg.Replace("LVR_", "");
                }
                else if (arg.Contains("LOCK_"))
                {
                    uploadConfigLockbits = arg.Replace("LOCK_", "");
                }

                // arduino IDE requesting upload

                if (arg.Contains("firmware_command.txt"))
                {
                    if (System.IO.File.Exists(arg))
                    {
                        string firmwareString = System.IO.File.ReadAllText(arg);
                        byte option = CalculateSelectedOptions(uploadConfigCpuFreq, uploadConfigLvr, uploadConfigLockbits);
                        Console.Error.WriteLine("Uploading (SerialPort: {0}, CPU Freq: {1}, LVR: {2}, Lock Bits: {3} = 0x{4:X2}) \n", uploadPort, uploadConfigCpuFreq, uploadConfigLvr, uploadConfigLockbits, option);
                        UploadViaSerial(string.Format("[{0:X2}]{1}", option, firmwareString), uploadPort);
                    }
                    else
                    {
                        Console.Error.WriteLine("Upload requested but firmware was not found, exiting");
                        Environment.Exit(1);
                    }

                }
            }
            return;
        }


        
        
    }
}
