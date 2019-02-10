unit CanTest;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, CanChanEx, Vcl.ExtCtrls,
  Vcl.Mask;

type
  TMainForm = class(TForm)
    onBus: TLabel;
    goOnBus: TButton;
    Send: TButton;
    Timer1: TTimer;
    Output: TListBox;
    GroupBox1: TGroupBox;
    CanDevices: TComboBox;
    AccelL: TEdit;
    AccelR: TEdit;
    BrakeF: TEdit;
    BrakeR: TEdit;
    ScrollSteering: TScrollBar;
    LabelAccelL: TLabel;
    LabelAccelR: TLabel;
    LabelBrakeF: TLabel;
    LabelBrakeR: TLabel;
    LabelSteering: TLabel;
    TS: TButton;
    RTDM: TButton;
    StopMotors: TButton;
    Steering: TMaskEdit;
    SendADC: TCheckBox;
    Label1: TLabel;
    Label2: TLabel;
    Label3: TLabel;
    TimeReceived: TLabel;
    Label5: TLabel;
    CanReceive118: TLabel;
    Label7: TLabel;
    Label8: TLabel;
    CanReceive47e: TLabel;
    CanReceive47f: TLabel;
    SendInverter: TButton;
    SendIMD: TButton;
    DriveMode: TComboBox;
    procedure FormKeyPress(Sender: TObject; var Key: Char);
    procedure FormCreate(Sender: TObject);
    procedure FormShow(Sender: TObject);
    procedure goOnBusClick(Sender: TObject);
    procedure SendClick(Sender: TObject);
    procedure CanDevicesChange(Sender: TObject);
    procedure ScrollSteeringChange(Sender: TObject);
    procedure SteeringChange(Sender: TObject);
    procedure TSClick(Sender: TObject);
    procedure RTDMClick(Sender: TObject);
    procedure StopMotorsClick(Sender: TObject);
    procedure SendInverterClick(Sender: TObject);
    procedure SendIMDClick(Sender: TObject);
  private
    { Private declarations }
    StartTime: TDateTime;
    CanChannel1: TCanChannelEx;
    InverterLStatus : Word;
    InverterLStatusRequest : Word;
    InverterRStatus : Word;
    InverterRStatusRequest : Word;
    procedure CanChannel1CanRx(Sender: TObject);
    function InterPolateSteering(SteeringAngle : Integer) : Word;
  public
    { Public declarations }
    procedure PopulateList;
  end;

var
  MainForm: TMainForm;

implementation

uses DateUtils, canlib;

{$R *.dfm}


procedure TMainForm.StopMotorsClick(Sender: TObject);
var
  msg: array[0..7] of byte;
begin
 msg[1] := 1;
  with CanChannel1 do begin
    Check(Write($612, msg, 1 { sizeof(msg) }, 0), 'Write failed');
  end;
end;

function TMainForm.InterPolateSteering(SteeringAngle : Integer) : Word;
   var
     i, dx, dy : Integer;

   const
     Input : Array[0..20] of Integer  = ( -100,-90,-80,-70,-60,-50,-40,-30,-20,-10,
                                  0,10,20,30,40,50,60,70,80,90,100 );

     Output  : Array[0..20] of Word = ( 1210,1270,1320,1360,1400,1450,1500,1540,1570,1630,
                                1680,1720,1770,2280,2700,3150,3600,4100,4700,5000,5500 );
    // output steering range. +-100%
   begin
      if SteeringAngle < Input[0] then
      begin
         result := Output[0];
         exit;
      end;

      if SteeringAngle > Input[Length(Input)-1] then
      begin
         result := Output[Length(Output)-1];
         exit;
      end;

    // loop through input values table till we find space where requested fits.
      i := 0;
      while Input[i+1] < SteeringAngle do inc(i);

      // interpolate
      dx := Input[i+1] - Input[i];
      dy := Output[i+1] - Output[i];
      Result := Round( Output[i] + ((SteeringANgle - Input[i]) * dy / dx) );

   end;


procedure TMainForm.PopulateList;
var
  i : Integer;
  p : AnsiString;
begin
  SetLength(p, 64);
  CanDevices.Items.clear;
  for i := 0 to CanChannel1.ChannelCount - 1 do begin
     CanDevices.Items.Add(CanChannel1.ChannelNames[i]);
  end;
  if CanDevices.Items.Count > 0 then
    CanDevices.ItemIndex := 0;
end;

procedure TMainForm.RTDMClick(Sender: TObject);
var
  msg: array[0..7] of byte;
begin
 msg[1] := 1;
  with CanChannel1 do begin
    Check(Write($611, msg, 1 { sizeof(msg) }, 0), 'Write failed');
  end;
end;

procedure TMainForm.ScrollSteeringChange(Sender: TObject);
begin
  Steering.Text := ScrollSteering.Position.ToString;
end;

procedure TMainForm.SendClick(Sender: TObject);
var
  W : Word;
  B : array[0..1] of byte absolute W;
  msg: array[0..7] of byte;  { this was defined as char in original test code
which is nowadays 16 bit in delphi. }
const
  SteeringID  = $601;
  AcceleratorID = $602;
  BrakeID = $603;
  DrivingModeID = $604;
  TemperatureID = $605;
  DrivingModeValue : array[0..7] of word = ( 100,300,470,690,900,1200,1600,1800 );
begin
  msg[1] := 1;
  with CanChannel1 do begin
    Check(Write(1, msg, 1 { sizeof(msg) }, 0), 'Write failed');   // this line was bugging out due to trying to send 16 bytes.
    if SendADC.checked then  // send our 'fake' adc data from form input.
    begin
      Output.Items.Add(TimeToStr(System.SysUtils.Now));
      W := InterPolateSteering(StrToInt(Steering.Text));
      msg[0] := B[1];
      msg[1] := B[0];
      Check(Write(SteeringID,msg,2,0), 'Write failed');

      W := StrToInt(AccelL.Text);
      msg[0] := B[1];
      msg[1] := B[0];
      W := StrToInt(AccelR.Text);
      msg[2] := B[1];
      msg[3] := B[0];
      Check(Write(AcceleratorID,msg,4,0), 'Write failed');

      W := StrToInt(BrakeF.Text);
      msg[0] := B[1];
      msg[1] := B[0];
      W := StrToInt(BrakeR.Text);
      msg[2] := B[1];
      msg[3] := B[0];
      Check(Write(BrakeID,msg,4,0), 'Write failed');

      W := DrivingModeValue[DriveMode.ItemIndex];
      msg[0] := B[1];
      msg[1] := B[0];
      Check(Write(DrivingModeID,msg,2,0), 'Write failed');

      W := 1;
      msg[0] := B[1];
      msg[1] := B[0];
      W := 1;
      msg[2] := B[1];
      msg[3] := B[0];
      Check(Write(TemperatureID,msg,4,0), 'Write failed');



    end;

  end;
end;

procedure TMainForm.SendIMDClick(Sender: TObject);
var
  msg: array[0..7] of byte;
begin
{  0x520,0,8 -> BMS_relay_status
				//	0x520,8,8 -> IMD_relay_status
				//	0x520,16,8 -> BSPD_relay_status        }
  msg[1] := 1;
  with CanChannel1 do begin
    Check(Write($520, msg, 3, 0), 'Write failed');
  end;

end;

procedure TMainForm.SendInverterClick(Sender: TObject);
var
  msg: array[0..7] of byte;
begin
      with CanChannel1 do
      begin
        case InverterLStatusRequest of
          128 : InverterLStatus := 64;

          6:  InverterLStatus := 49;

          7:  InverterLStatus := 51;

          15:  begin
             {     if InverterLStatus = 55 then
                     InverterLStatus := 104    -- shutdown?
                  else         }
                      InverterLStatus := 55;
                end;
        else
             //     msg[0] := 0;
             //     msg[1] := 0;
        end;


        msg[0] := InverterLStatus;
        msg[1] := 22;

        Check(Write($1fe,msg,2,0), 'Write failed');

        case InverterRStatusRequest of
          128 : InverterRStatus := 64;

          6:  InverterRStatus := 49;

          7:  InverterRStatus := 51;

          15:  begin
             {     if InverterRStatus = 55 then
                     InverterRStatus := 104
                  else }
                      InverterRStatus := 55;
                end;
        else
             //     msg[0] := 0;
             //     msg[1] := 0;
        end;

        msg[0] := InverterRStatus;
        msg[1] := 22;

        Output.Items.Add( TimeToStr(System.SysUtils.Now) +
        ' InverterL&R 1FE sent status '+ IntToStr(InverterLStatus) + ' '
        + IntToStr(InverterRStatus));


        Check(Write($1ff,msg,2,0), 'Write failed');
      end;
end;

procedure TMainForm.SteeringChange(Sender: TObject);
begin
  ScrollSteering.position := StrToInt(Steering.Text);
 // Output.Items.Add(IntToStr(InterPolateSteering(StrToInt((Steering.Text)))));
end;

procedure TMainForm.TSClick(Sender: TObject);
var
  msg: array[0..7] of byte;
begin
 msg[1] := 1;
  with CanChannel1 do begin
    Check(Write($610, msg, 1 { sizeof(msg) }, 0), 'Write failed');
  end;
end;

procedure TMainForm.goOnBusClick(Sender: TObject);
var
  msg: array[0..7] of byte;
begin
  with CanChannel1 do begin
    if not Active then begin
      Bitrate := canBITRATE_1M;
      Open;
      CanChannel1.OnCanRx := CanChannel1CanRx;
      BusActive := true;
      CanDevices.Enabled := false;
      onBus.Caption := 'On bus';
      goOnBus.Caption := 'Go off bus';
      StartTime := Now;
      Send.Enabled := true;
      with CanChannel1 do begin
        if SendADC.checked then
        begin
          msg[0] := 1;
          Check(Write($600, msg, 1, 0), 'Write failed'); // tell ECU to use can 'ADC' values for testing.
        end
        else
        begin
          msg[0] := 0;
          Check(Write($600, msg, 1, 0), 'Write failed'); // tell ECU to use can 'ADC' values for testing.
        end
      end
    end else begin
      BusActive := false;
      onBus.Caption := 'Off bus';
      goOnBus.Caption := 'Go on bus';
      CanDevices.Enabled := true;
      Send.Enabled := false;
      Close;
    end;

  //  if Active then Label1.Caption := 'Active' else Label1.Caption := 'Inactive';

  end;
end;

procedure TMainForm.CanDevicesChange(Sender: TObject);
begin
   CanChannel1.Channel := CanDevices.ItemIndex;
end;

procedure TMainForm.FormCreate(Sender: TObject);
begin
  CanChannel1 := TCanChannelEx.Create(Self);
  CanChannel1.Channel := 0;
  Output.clear;
end;

procedure TMainForm.FormKeyPress(Sender: TObject; var Key: Char);
begin
  if Key = chr(27) then Close; //  close window on hitting Esc
end;

procedure TMainForm.FormShow(Sender: TObject);
begin
  PopulateList;
end;

procedure TMainForm.CanChannel1CanRx(Sender: TObject);
var
  dlc, flag, time: cardinal;
  msg: array[0..7] of byte;
  id: longint;
  CurTime : TDateTime;
begin
  Output.Items.BeginUpdate;
  with CanChannel1 do begin
    while Read(id, msg, dlc, flag, time) >= 0 do begin
      case id of
        $100 : Output.Items.Add(Format('Id=%d Len=%d %d', [id, dlc, msg[0]]));

        $47E : begin
                 InverterLStatusRequest := msg[0];
                 CanReceive47e.Caption := IntToStr(msg[0]) + ' ' + IntToStr(msg[1])
                                + ' ' +  IntToStr(msg[2]) + ' ' + IntToStr(msg[3]) + ' ' + TimeToStr(System.SysUtils.Now);
               end;

        $47F : begin
                 InverterRStatusRequest := msg[0];
                 CanReceive47f.Caption := IntToStr(msg[0]) + ' ' + IntToStr(msg[1])
                                +  ' '  +  IntToStr(msg[2]) + ' ' + IntToStr(msg[3]) + ' ' + TimeToStr(System.SysUtils.Now);
               end;

        $118 : begin
                 CanReceive118.Caption := IntToStr(msg[0]) + ' ' + IntToStr(msg[1]);
                 TimeReceived.Caption := TimeToStr(System.SysUtils.Now);
               end;
      else

      end;
    end;
  end;
  Output.Items.EndUpdate;
end;

end.
