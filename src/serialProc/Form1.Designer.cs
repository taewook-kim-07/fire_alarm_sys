namespace serialProc
{
    partial class Form1
    {
        /// <summary>
        /// 필수 디자이너 변수입니다.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// 사용 중인 모든 리소스를 정리합니다.
        /// </summary>
        /// <param name="disposing">관리되는 리소스를 삭제해야 하면 true이고, 그렇지 않으면 false입니다.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form 디자이너에서 생성한 코드

        /// <summary>
        /// 디자이너 지원에 필요한 메서드입니다. 
        /// 이 메서드의 내용을 코드 편집기로 수정하지 마세요.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            this.comboBox = new System.Windows.Forms.ComboBox();
            this.connect_button = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.InputTextBox = new System.Windows.Forms.TextBox();
            this.Send_Button = new System.Windows.Forms.Button();
            this.comrefresh_button = new System.Windows.Forms.Button();
            this.serialPort = new System.IO.Ports.SerialPort(this.components);
            this.ws_init_timer = new System.Windows.Forms.Timer(this.components);
            this.logTextBox = new System.Windows.Forms.RichTextBox();
            this.SuspendLayout();
            // 
            // comboBox
            // 
            this.comboBox.FormattingEnabled = true;
            this.comboBox.Location = new System.Drawing.Point(12, 36);
            this.comboBox.Name = "comboBox";
            this.comboBox.Size = new System.Drawing.Size(139, 20);
            this.comboBox.TabIndex = 0;
            // 
            // connect_button
            // 
            this.connect_button.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.connect_button.Location = new System.Drawing.Point(12, 62);
            this.connect_button.Name = "connect_button";
            this.connect_button.Size = new System.Drawing.Size(139, 22);
            this.connect_button.TabIndex = 1;
            this.connect_button.Text = "연결";
            this.connect_button.UseVisualStyleBackColor = true;
            this.connect_button.Click += new System.EventHandler(this.connect_button_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 21);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(90, 12);
            this.label1.TabIndex = 2;
            this.label1.Text = "COM 포트 선택";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(155, 21);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(30, 12);
            this.label2.TabIndex = 2;
            this.label2.Text = "LOG";
            // 
            // InputTextBox
            // 
            this.InputTextBox.Location = new System.Drawing.Point(12, 154);
            this.InputTextBox.Name = "InputTextBox";
            this.InputTextBox.Size = new System.Drawing.Size(139, 21);
            this.InputTextBox.TabIndex = 4;
            // 
            // Send_Button
            // 
            this.Send_Button.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.Send_Button.Location = new System.Drawing.Point(12, 179);
            this.Send_Button.Name = "Send_Button";
            this.Send_Button.Size = new System.Drawing.Size(139, 22);
            this.Send_Button.TabIndex = 5;
            this.Send_Button.Text = "SEND (DEBUG)";
            this.Send_Button.UseVisualStyleBackColor = true;
            this.Send_Button.Click += new System.EventHandler(this.Send_Button_Click);
            // 
            // comrefresh_button
            // 
            this.comrefresh_button.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.comrefresh_button.Location = new System.Drawing.Point(12, 90);
            this.comrefresh_button.Name = "comrefresh_button";
            this.comrefresh_button.Size = new System.Drawing.Size(139, 22);
            this.comrefresh_button.TabIndex = 1;
            this.comrefresh_button.Text = "COM 포트 목록 갱신";
            this.comrefresh_button.UseVisualStyleBackColor = true;
            this.comrefresh_button.Click += new System.EventHandler(this.comrefresh_button_Click);
            // 
            // ws_init_timer
            // 
            this.ws_init_timer.Interval = 5000;
            this.ws_init_timer.Tick += new System.EventHandler(this.ws_init_timer_Tick);
            // 
            // logTextBox
            // 
            this.logTextBox.Location = new System.Drawing.Point(157, 36);
            this.logTextBox.Name = "logTextBox";
            this.logTextBox.ReadOnly = true;
            this.logTextBox.Size = new System.Drawing.Size(415, 167);
            this.logTextBox.TabIndex = 6;
            this.logTextBox.Text = "";
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(7F, 12F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(584, 215);
            this.Controls.Add(this.logTextBox);
            this.Controls.Add(this.Send_Button);
            this.Controls.Add(this.InputTextBox);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.comrefresh_button);
            this.Controls.Add(this.connect_button);
            this.Controls.Add(this.comboBox);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.Name = "Form1";
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
            this.Text = "UART - WebSocket :: 전자응용설계 02분반 2조 ::";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.Load += new System.EventHandler(this.Form1_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox comboBox;
        private System.Windows.Forms.Button connect_button;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox InputTextBox;
        private System.Windows.Forms.Button Send_Button;
        private System.Windows.Forms.Button comrefresh_button;
        private System.IO.Ports.SerialPort serialPort;
        private System.Windows.Forms.Timer ws_init_timer;
        private System.Windows.Forms.RichTextBox logTextBox;
    }
}

