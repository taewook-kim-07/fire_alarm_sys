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
using System.Net.WebSockets;
using System.Threading;

namespace serialProc
{
    public partial class Form1 : Form
    {
        Thread WebsocketTask;
        ClientWebSocket ws;

        string Title = "UART - WebSocket :: 전자응용설계 02분반 2조 ::";
        bool SerialConnected = false;

        public Form1()
        {
            InitializeComponent();
        }

#region Websocket
        private void ws_init_timer_Tick(object sender, EventArgs e)
        {
            ws_init_timer.Enabled = false;
            if (WebsocketTask != null && WebsocketTask.IsAlive) WebsocketTask.Abort();
            WebsocketTask = new Thread(websocket_Init);
            WebsocketTask.Start();
        }

        private void websocket_Init()
        {
            Task t = task_WebSocketClient();
            t.Wait();
        }

        private async Task task_WebSocketClient()
        {
           // ws.CloseAsync
            using (ws = new ClientWebSocket())
            {
                Uri serverUri = new Uri("wss://wss.example.com");
                try {
                    await ws.ConnectAsync(serverUri, CancellationToken.None); 
                } catch (Exception e) { }

                if (ws.State == WebSocketState.Open)
                {
                    Invoke(new Action(delegate ()
                    {
                        this.Text = Title;
                        LogAppend("Websocket 서버와 연결 성공");
                    }));
                    ws_send("controlnet_init");

                    while (ws.State == WebSocketState.Open)
                    {
                        ArraySegment<byte> bytesReceived = new ArraySegment<byte>(new byte[1024]);
                        try
                        { 
                            WebSocketReceiveResult result = await ws.ReceiveAsync(bytesReceived, CancellationToken.None);
                            string encoded_msg = Encoding.UTF8.GetString(bytesReceived.Array, 0, result.Count);
                            LogAppend(string.Format("[WS, Receive] {0}", encoded_msg));

                            serial_send(encoded_msg);
                        } catch (Exception e) { }
                    }
                }
                Invoke(new Action(delegate () {
                    this.Text = Title + " [서버 연결 실패]";
                    LogAppend("Websocket 서버와 연결 실패!!...5초 후 재연결");
                    ws_init_timer.Enabled = true;
                }));
            }
        }

        private bool ws_send(string message)
        {
            if(ws.State == WebSocketState.Open)
            {
                ArraySegment<byte> bytesToSend = new ArraySegment<byte>(Encoding.UTF8.GetBytes(message));
                ws.SendAsync(bytesToSend, WebSocketMessageType.Text, true, CancellationToken.None).Wait();

                LogAppend(string.Format("[WS, Send] {0}", message));
                return true;
            }
            return false;
        }
 #endregion

        private void Form1_Load(object sender, EventArgs e)
        {
            comboBox.DataSource = SerialPort.GetPortNames();    // COM 포트 목록 이름을 가져옴
            ws_init_timer_Tick(sender, e);                      // Timer Enable = false인 상태로 호출
        }
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            WebsocketTask.Abort();
        }

        private void LogAppend(string message)
        {
            Invoke(new Action(delegate () { 
                logTextBox.AppendText(string.Format("{0}\n", message));
                logTextBox.ScrollToCaret();
            }));
        }

        private void connect_button_Click(object sender, EventArgs e)
        {
            if (comboBox.Text.Trim().Length == 0) return;

            bool closeflag = false;

            MSGQueue = string.Empty;

            if (!serialPort.IsOpen)  //시리얼포트가 열려 있지 않으면
            {
                try
                {
                    serialPort.PortName = comboBox.Text;  //콤보박스의 선택된 COM포트명을 시리얼포트명으로 지정
                    serialPort.BaudRate = 9600;  //보레이트 변경이 필요하면 숫자 변경하기
                    serialPort.DataBits = 8;
                    serialPort.StopBits = StopBits.One;
                    serialPort.Parity = Parity.None;
                    serialPort.DataReceived += new SerialDataReceivedEventHandler(serialPort1_DataReceived); //이것이 꼭 필요하다

                    serialPort.Open();  //시리얼포트 열기
                    LogAppend(string.Format("{0} 포트가 열렸습니다.", serialPort.PortName));
                    connect_button.Text = "연결 해제";
                    SerialConnected = true;
                }
                catch (Exception ex) { closeflag = true; }
            }
            else
            {
                closeflag = true;
            }
            if(closeflag)
            {
                serialPort.Close();  //시리얼포트 닫기
                LogAppend(string.Format("{0} 포트가 닫혔습니다.", serialPort.PortName));
                connect_button.Text = "연결";
                SerialConnected = false;
            }
        }

        private void comrefresh_button_Click(object sender, EventArgs e)
        {
            comboBox.DataSource = SerialPort.GetPortNames();
            LogAppend("포트 목록이 갱신되었습니다.");
        }

        private void Send_Button_Click(object sender, EventArgs e)
        {
            ws_send(InputTextBox.Text);
            serial_send(InputTextBox.Text);
        }

        private bool serial_send(string message)
        {
            if (!SerialConnected) return false;
            if (!serialPort.IsOpen) { SerialConnected = false; return false; }
            serialPort.WriteLine(message);// + '\0');

            LogAppend(string.Format("[Serial, Send] {0}", message));
            return true;
        }

        private void serialPort1_DataReceived(object sender, SerialDataReceivedEventArgs e)
        {
            this.Invoke(new EventHandler(MySerialReceived));
        }

        string MSGQueue = string.Empty;
        private void MySerialReceived(object s, EventArgs e)  //여기에서 수신 데이타를 사용자의 용도에 따라 처리한다.
        {
            string message = string.Empty;
            int BufferSize = serialPort.BytesToRead;
            if(BufferSize != 0)
            {
                byte[] Buffer = new byte[BufferSize];
                serialPort.Read(Buffer, 0, BufferSize);

                message = Encoding.Default.GetString(Buffer);
                MSGQueue = string.Concat(MSGQueue, message);
                for (int i=BufferSize-1; i>=0; i--)
                {
                    if (Buffer[i] == 20) // device control 4
                    {
                        LogAppend(string.Format("[Serial, Receive] {0}", MSGQueue));
                        ws_send(MSGQueue);
                        MSGQueue = string.Empty;
                        break;
                    }
                }
            }
        }

    }
}
