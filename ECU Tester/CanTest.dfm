object MainForm: TMainForm
  Left = 0
  Top = 0
  Caption = 'MainForm'
  ClientHeight = 402
  ClientWidth = 814
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  OnCreate = FormCreate
  OnKeyPress = FormKeyPress
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 14
  object GroupBox1: TGroupBox
    Left = 8
    Top = 8
    Width = 798
    Height = 385
    Caption = 'GroupBox1'
    TabOrder = 0
    object onBus: TLabel
      Left = 16
      Top = 48
      Width = 29
      Height = 13
      Caption = 'onBus'
    end
    object LabelAccelL: TLabel
      Left = 16
      Top = 171
      Width = 30
      Height = 13
      Caption = 'AccelL'
    end
    object LabelAccelR: TLabel
      Left = 16
      Top = 198
      Width = 32
      Height = 13
      Caption = 'AccelR'
    end
    object LabelBrakeF: TLabel
      Left = 16
      Top = 217
      Width = 33
      Height = 13
      Caption = 'BrakeF'
    end
    object LabelBrakeR: TLabel
      Left = 16
      Top = 252
      Width = 34
      Height = 13
      Caption = 'BrakeR'
    end
    object LabelSteering: TLabel
      Left = 18
      Top = 280
      Width = 40
      Height = 13
      Caption = 'Steering'
    end
    object Label1: TLabel
      Left = 536
      Top = 65
      Width = 24
      Height = 13
      Caption = '$47E'
    end
    object Label2: TLabel
      Left = 536
      Top = 84
      Width = 24
      Height = 13
      Caption = '$47F'
    end
    object Label3: TLabel
      Left = 536
      Top = 46
      Width = 26
      Height = 13
      Caption = 'Time:'
    end
    object TimeReceived: TLabel
      Left = 592
      Top = 46
      Width = 6
      Height = 13
      Caption = '0'
    end
    object Label5: TLabel
      Left = 536
      Top = 103
      Width = 24
      Height = 13
      Caption = '$118'
    end
    object CanReceive118: TLabel
      Left = 592
      Top = 103
      Width = 75
      Height = 13
      Caption = 'CanReceive118'
    end
    object Label7: TLabel
      Left = 536
      Top = 179
      Width = 31
      Height = 13
      Caption = 'Label1'
    end
    object Label8: TLabel
      Left = 536
      Top = 198
      Width = 31
      Height = 13
      Caption = 'Label1'
    end
    object CanReceive47e: TLabel
      Left = 592
      Top = 65
      Width = 75
      Height = 13
      Caption = 'CanReceive47e'
    end
    object CanReceive47f: TLabel
      Left = 592
      Top = 84
      Width = 73
      Height = 13
      Caption = 'CanReceive47f'
    end
    object CanDevices: TComboBox
      Left = 3
      Top = 20
      Width = 145
      Height = 22
      Style = csDropDownList
      TabOrder = 0
      OnChange = CanDevicesChange
    end
    object Output: TListBox
      Left = 191
      Top = 20
      Width = 322
      Height = 238
      ItemHeight = 14
      TabOrder = 1
    end
    object Send: TButton
      Left = 3
      Top = 98
      Width = 75
      Height = 25
      Caption = 'Send'
      Enabled = False
      TabOrder = 2
      OnClick = SendClick
    end
    object goOnBus: TButton
      Left = 3
      Top = 67
      Width = 75
      Height = 25
      Caption = 'Go on bus'
      TabOrder = 3
      OnClick = goOnBusClick
    end
    object AccelL: TEdit
      Left = 64
      Top = 168
      Width = 121
      Height = 21
      NumbersOnly = True
      TabOrder = 4
      Text = '0'
    end
    object AccelR: TEdit
      Left = 64
      Top = 195
      Width = 121
      Height = 21
      NumbersOnly = True
      TabOrder = 5
      Text = '0'
    end
    object BrakeF: TEdit
      Left = 64
      Top = 222
      Width = 121
      Height = 21
      NumbersOnly = True
      TabOrder = 6
      Text = '0'
    end
    object BrakeR: TEdit
      Left = 64
      Top = 249
      Width = 121
      Height = 21
      NumbersOnly = True
      TabOrder = 7
      Text = '0'
    end
    object ScrollSteering: TScrollBar
      Left = 64
      Top = 276
      Width = 449
      Height = 17
      Min = -100
      PageSize = 0
      TabOrder = 8
      OnChange = ScrollSteeringChange
    end
    object TS: TButton
      Left = 40
      Top = 344
      Width = 75
      Height = 25
      Caption = 'TS'
      TabOrder = 9
      OnClick = TSClick
    end
    object RTDM: TButton
      Left = 144
      Top = 344
      Width = 75
      Height = 25
      Caption = 'RTDM'
      TabOrder = 10
      OnClick = RTDMClick
    end
    object StopMotors: TButton
      Left = 248
      Top = 344
      Width = 75
      Height = 25
      Caption = 'Stop Motors'
      TabOrder = 11
      OnClick = StopMotorsClick
    end
    object Steering: TMaskEdit
      Left = 64
      Top = 299
      Width = 121
      Height = 21
      TabOrder = 12
      Text = '0'
      OnChange = SteeringChange
    end
    object SendADC: TCheckBox
      Left = 64
      Top = 145
      Width = 97
      Height = 17
      Caption = 'SendADC'
      Checked = True
      State = cbChecked
      TabOrder = 13
    end
    object SendInverter: TButton
      Left = 568
      Top = 276
      Width = 121
      Height = 25
      Caption = 'Send Inverter Status'
      TabOrder = 14
      OnClick = SendInverterClick
    end
  end
  object Timer1: TTimer
    Left = 120
    Top = 88
  end
end
