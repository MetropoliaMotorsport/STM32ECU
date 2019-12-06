unit CanChanEx;
(*
**                        Copyright 1998 by KVASER AB
**                  P.O Box 4076 SE-51104 KINNAHULT, SWEDEN
**            E-mail: support@kvaser.com   WWW: http://www.kvaser.se
**
** This software is furnished under a license and may be used and copied
** only in accordance with the terms of such license.
**
*)
interface

uses
  SysUtils, Windows, Messages, Classes, Graphics, Controls, Forms, CANLIB;

type
   { Error in the DLL }
  ECanError = class(Exception)
  public
    ErrCode: integer;
    ErrText: string[50];
    constructor Create(text, subtext: string; code: integer);
  end;

   { Error detected in this component }
  ECanChan = class(Exception);

  TCanWriteEvent = procedure(Sender: TObject;
    id: Longint; 
    msg: Pointer; 
    dlc, flag: Integer) of object;

  { Channel options }
  TCanChanOption = (ccNotExclusive, ccNoVirtual, ccAcceptLargeDLC);
  TCanChanOptions = set of TCanChanOption;

  { Circuit driver options }
  TCanChanDrivers = (ccNormal, ccSilent);

  { Notification options }
  TCanChanNotifyFlag = (ccNotifyRx, ccNotifyTx, ccNotifyStatus);
  TCanChanNotifyFlags = set of TCanChanNotifyFlag;

  TCanChannelEx = class(TComponent)
  private
    FDllFailed: Boolean;
    FWinHandle: HWND;
    FCanHnd: canHandle;
    FActive: boolean;
    FBusActive: boolean;
    FOnCanRx: TNotifyEvent;
    FOnCanTx: TNotifyEvent;
    FOnCanErr: TNotifyEvent;
    FOnWrite: TCanWriteEvent;
    FOnStateChanged: TNotifyEvent;
    FChannel: Integer;
    FOptions: TCanChanOptions;
    FDriver: TCanChanDrivers;

    FBitRate: Longint;
    FTseg1: Integer;
    FTseg2: Integer;
    FSjw: Integer;
    FNoSamp: Integer;

    FNotifyFlags: TCanChanNotifyFlags;

    procedure MessageHandler(var msg: TMessage);
    procedure SetBusActive(aValue: Boolean);
    procedure SetActive(aValue: Boolean);
    procedure SetNotifyFlags(aValue: TCanChanNotifyFlags);
    procedure SetWinHandle(aValue: HWND);
    procedure SetOnCan(Index: Integer; aProc: TNotifyEvent);
    function GetOnCan(Index: Integer): TNotifyEvent;
    procedure SetChannel(channel: integer);
    procedure SetOptions(options: TCanChanOptions);
    procedure SetDriver(driver: TCanChanDrivers);
    procedure SetBusParam(index: integer; value: Integer);
    function GetBusParam(index: Integer): Integer;
    procedure SetBitRate(value: Longint);

    procedure AssertActive;
    procedure AssertNotActive;
    procedure AssertNotBusActive;
    procedure UpdateCANCircuitParameters;

    function GetChannelStatus: Cardinal;
    function GetNow: Cardinal;
    function GetTransmitErrors: Cardinal;
    function GetReceiveErrors: Cardinal;
    function GetOverruns: Cardinal;
    function GetChannelCount: Cardinal;
    function GetChannelNames(idx: Integer): string;
    function GetChannelName: String;

  protected
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    procedure Check(result: Integer; text: string);
    procedure Open;
    procedure Close;
    procedure BusOn;
    procedure BusOff;
    function SetHardwareFilters(id: Longint; flag: Cardinal): integer;
    function Write(id: Longint; var msg; dlc, flags: Cardinal): integer;
    function WriteSync(timeout: Cardinal): integer;
    function Read(var id: Longint; var msg; var dlc, flags: Cardinal; var time: Cardinal): integer;
    function ReadWait(var id: Longint; var msg; var dlc, flags: Cardinal; var time: Cardinal; timeout: Cardinal): integer;
    function ReadSpecific(id: Longint; var msg; var dlc, flags: Cardinal; var time: Cardinal): integer;
    function ReadSync(timeout: Cardinal): integer;
    function ReadSyncSpecific(id: Longint; timeout: Cardinal): integer;
    function ReadSpecificSkip(id: Longint; var msg; var dlc, flags: Cardinal; var time: Cardinal): Integer;
    function WaitForEvent(timeout: Cardinal): Integer;
{ qqq todo:
canIoCtl
canGetChannelData
canSetBusParamsC200
}

    { Runtime-only properties }
    property CanHnd: canHandle read FCanHnd;
    property WinHandle: HWND read FWinHandle write setWinHandle;
    property LoadFailed: Boolean read FDllFailed;

    property Now: Cardinal read GetNow;
    property TransmitErrors: Cardinal read GetTransmitErrors;
    property ReceiveErrors: Cardinal read GetReceiveErrors;
    property Overruns: Cardinal read GetOverruns;
    property ChannelCount: Cardinal read GetChannelCount;
    property ChannelNames[idx: Integer]: string read GetChannelNames;
    property ChannelName: String read GetChannelName;
    property ChannelStatus: Cardinal read GetChannelStatus;

  published
    property Channel: Integer read FChannel write SetChannel;
    property Options: TCanChanOptions read FOptions write SetOptions;
    property Active: Boolean read FActive write SetActive stored False;
    property BitRate: Longint read FBitRate write SetBitRate;
    property TSeg1: Integer index 1 read GetBusParam write SetBusParam;
    property TSeg2: Integer index 2 read GetBusParam write SetBusParam;
    property SJW: Integer index 3 read GetBusParam write SetBusParam;
    property Samples: Integer index 4 read GetBusParam write SetBusParam default 1;
    property Driver: TCanChanDrivers read FDriver write SetDriver default ccNormal;
    property BusActive: Boolean read FBusActive write SetBusActive stored False;
    property OnCanRx: TNotifyEvent index 1 read getOnCan write setOnCan;
    property OnCanTx: TNotifyEvent index 2 read getOnCan write setOnCan;
    property OnStateChanged: TNotifyEvent index 3 read getOnCan write setOnCan;
    property OnWrite: TCanWriteEvent read FOnWrite write FOnWrite;
  end;

procedure Register;

implementation

procedure Register;
begin
  RegisterComponents('Kvaser', [TCanChannelEx]);
end;

function N2I(n: TCanChanNotifyFlags): Longint;
begin
  Result := 0;
  if ccNotifyRx in n then Result := Result or canNOTIFY_RX;
  if ccNotifyTx in n then Result := Result or canNOTIFY_TX;
  if ccNotifyStatus in n then Result := Result or canNOTIFY_STATUS;
end;

function I2N(n: Longint): TCanChanNotifyFlags;
begin
  Result := [];
  if (n and canNOTIFY_RX) <> 0 then Include(Result, ccNotifyRx);
  if (n and canNOTIFY_TX) <> 0 then Include(Result, ccNotifyTx);
  if (n and canNOTIFY_STATUS) <> 0 then Include(Result, ccNotifyStatus);
end;

constructor ECanError.Create(text, subtext: string; code: integer);
begin
  inherited Create(text);
  ErrText := ShortString(subtext);
  ErrCode := code;
end;

{
| Create the component. Initialize the DLL. Don't open the
| circuit. Stay calm.
}

constructor TCanChannelEx.Create(AOwner: TComponent);
var tmp: Integer;
begin
  inherited Create(AOwner);

  FCanHnd := canINVALID_HANDLE;
  FDriver := ccNormal;
  FBitRate := 1000000;
  FTseg1 := 4;
  FTseg2 := 3;
  FSjw := 1;
  FNoSamp := 1;

  if not (csDesigning in ComponentState) then begin
    try
      canInitializeLibrary;
      FWinHandle := Classes.AllocateHWnd(MessageHandler);
      FDllFailed := False;
    except
      FDllFailed := True;
    end;
    tmp := 0;
    if (canGetNumberOfChannels(tmp) <> canOK) or (tmp < 1) then FDllFailed := True;
  end;
end;

{
| Destroy the component.
}

destructor TCanChannelEx.Destroy;
begin
  if not (csDesigning in ComponentState) then begin
    Classes.DeallocateHWnd(FWinHandle);
    if FActive and (not FDllFailed) then Close;
  end;
  inherited Destroy;
end;

procedure TCanChannelEx.SetChannel(channel: integer);
begin
  AssertNotActive;
  FChannel := channel;
  if Assigned(FOnStateChanged) then FOnStateChanged(Self);
end;

procedure TCanChannelEx.SetOptions(options: TCanChanOptions);
begin
  AssertNotActive;
  FOptions := options;
end;

procedure TCanChannelEx.SetDriver(driver: TCanChanDrivers);
begin
  AssertNotBusActive;
  FDriver := driver;
end;

procedure TCanChannelEx.SetBitRate(value: Longint);
var tseg1, tseg2, sjw, nosamp, syncmode, stat: Cardinal;
begin
  AssertNotBusActive;
  if value < 0 then begin
    stat := canTranslateBaud(value, tseg1, tseg2, sjw, nosamp, syncmode);
    Check(stat, 'Illegal Bus Parameter Value');
    FTseg1 := tseg1;
    FTseg2 := tseg2;
    FSjw := sjw;
    FNoSamp := nosamp;
  end;
  FBitRate := value;
  if Assigned(FOnStateChanged) then FOnStateChanged(Self);
end;

procedure TCanChannelEx.SetBusParam(index: integer; value: Integer);
begin
  AssertNotBusActive;
  case index of
    1: FTseg1 := value;
    2: FTseg2 := value;
    3: FSjw := value;
    4: FNoSamp := value;
  end;
  if Assigned(FOnStateChanged) then FOnStateChanged(Self);
end;

function TCanChannelEx.GetBusParam(index: Integer): integer;
begin
  case index of
    1: Result := FTseg1;
    2: Result := FTseg2;
    3: Result := FSjw;
    4: Result := FNoSamp;
    else
      raise ECanChan.Create('Illegal index');
  end;
end;

{
| Check the error code; raise appropriate exception.
}

procedure TCanChannelEx.Check(result: integer; text: string);
var
  tmp: AnsiString;
  s: string;
begin
  if result <> canOK then begin
    if Assigned(canGetErrorText) then begin
      setlength(tmp, 50);
      canGetErrorText(result, @tmp[1], sizeof(tmp));
      s := string(tmp);
    end else begin
      s := Format('Error number %d - no text available.', [result]);
    end;
    raise ECanError.Create(text, s, result);
  end;
end;

{
| Raise an exception if the channel is inactive.
}

procedure TCanChannelEx.AssertActive;
begin
  if not FActive then raise ECanChan.Create('Channel is not active');
end;

{
| Raise an exception if the channel is active.
}

procedure TCanChannelEx.AssertNotActive;
begin
  if FActive then raise ECanChan.Create('Channel is active');
end;

{
| Raise an exception if the channel is off-bus. (not used.)
}
(*
procedure TCanChannelEx.AssertBusActive;
begin
   if not FBusActive then raise ECanChan.Create('Channel is off-bus');
end;
*)

{
| Raise an exception if the channel is on-bus.
}

procedure TCanChannelEx.AssertNotBusActive;
begin
  if FBusActive then raise ECanChan.Create('Channel is on-bus');
end;

{
| Handle notification messages.
}

procedure TCanChannelEx.MessageHandler(var msg: TMessage);
var stat: Cardinal;
begin
  if not FActive then Exit;
  if msg.Msg = WM__CANLIB then begin
    case TWMCan(msg).minorMsg of
      canEVENT_RX:
        begin
          if Assigned(FOnCanRx) then FOnCanRx(Self);
        end;

      canEVENT_TX:
        begin
          if Assigned(FOnCanTx) then FOnCanTx(Self);
        end;

      canEVENT_ERROR:
        begin
          if Assigned(FOnCanErr) then FOnCanErr(Self);
        end;

      canEVENT_STATUS:
        begin
          canReadStatus(FCanHnd, stat);
          if (stat and canSTAT_BUS_OFF) <> 0 then FBusActive := False;
          if Assigned(FOnStateChanged) then FOnStateChanged(Self);
        end;
    end;
  end;
end;

procedure TCanChannelEx.SetActive(aValue: boolean);
begin
  if (csDesigning in ComponentState) then Exit;
  if aValue then Open else Close;
  if Assigned(FOnStateChanged) then FOnStateChanged(Self);
end;

procedure TCanChannelEx.UpdateCANCircuitParameters;
var stat: Longint;
  driver: Integer;
begin
  if csDesigning in ComponentState then Exit;
  if FDllFailed then Exit;
  stat := canSetBusParams(FCanHnd, FBitRate, FTseg1, FTseg2, FSjw, FNoSamp, 0);

  if stat < 0 then begin
    canClose(FCanHnd);
    FCanHnd := canINVALID_HANDLE;
    Check(stat, 'Illegal bus parameters');
  end;

  driver := canDRIVER_NORMAL;
  case FDriver of
    ccNormal: driver := canDRIVER_NORMAL;
    ccSilent: driver := canDRIVER_SILENT;
  end;

  stat := canSetBusOutputControl(FCanHnd, driver);
  if stat < 0 then begin
    canClose(FCanHnd);
    FCanHnd := canINVALID_HANDLE;
    Check(stat, 'Illegal Bus Driver Type');
  end;
end;

{
| Open the channel. Set bus parameters. Stay off-bus.
}

procedure TCanChannelEx.Open;
var
  result: Integer;
  hnd: canHandle;
  flags: integer;
begin
  if csDesigning in ComponentState then Exit;
  if FActive or FDllFailed then Exit;

  flags := canOPEN_EXCLUSIVE + canOPEN_ACCEPT_VIRTUAL;
  if ccNotExclusive in FOptions then flags := flags and not canOPEN_EXCLUSIVE;
  if ccNoVirtual in FOptions then flags := flags and not canOPEN_ACCEPT_VIRTUAL;
  if ccAcceptLargeDLC in FOptions then flags := flags or canOPEN_ACCEPT_LARGE_DLC;

  hnd := canOpenChannel(FChannel, flags);
  if hnd < 0 then begin
    Check(hnd, 'canOpenChannel failed');
  end;

  FCanHnd := hnd;
  UpdateCANCircuitParameters;

  result := canSetNotify(hnd, FWinHandle, N2I(FNotifyFlags));
  if result < 0 then begin
    canClose(hnd);
      {Throw exception }
    Check(result, 'SetNotify failed in Open');
  end;

  FCanHnd := hnd;
  FActive := True;
  FBusActive := False;
  if Assigned(FOnStateChanged) then FOnStateChanged(Self);
end;

{
| Close the channel.
}

procedure TCanChannelEx.Close;
var
  result: Integer;
begin
  if csDesigning in ComponentState then Exit;
  if (not FActive) or FDllFailed then Exit;
  Result := canClose(FCanHnd);
  if not (csDestroying in Componentstate) then begin
    FCanHnd := canINVALID_HANDLE;
    Check(result, 'canClose failed');
    FActive := False;
    FBusActive := False;
    if Assigned(FOnStateChanged) then FOnStateChanged(Self);
  end;
end;

procedure TCanChannelEx.BusOn;
begin
   BusActive := True;
end;

procedure TCanChannelEx.BusOff;
begin
   BusActive := False;
end;

function TCanChannelEx.Write(id: Longint;
  var msg;
  dlc, flags: Cardinal): integer;
begin
  Result := canWrite(FCanHnd, id, @msg, dlc, flags);
   { "Local echo" - call OnWrite }
  if (Result = canOK) and Assigned(FOnWrite) then
    FOnWrite(self, id, PChar(@msg), dlc, flags);
end;

function TCanChannelEx.WriteSync(timeout: Cardinal): integer;
begin
  Result := canWriteSync(FCanHnd, timeout);
end;

function TCanChannelEx.Read(var id: Longint;
  var msg;
  var dlc, flags: Cardinal;
  var time: Cardinal): integer;
begin
  Result := canRead(FCanHnd, id, @msg, dlc, flags, time);
end;

function TCanChannelEx.ReadWait(var id: Longint;
  var msg;
  var dlc, flags: Cardinal;
  var time: Cardinal; timeout: Cardinal): integer;
begin
  Result := canReadWait(FCanHnd, id, @msg, dlc, flags, time, timeout);
end;

function TCanChannelEx.ReadSpecific(id: Longint;
  var msg;
  var dlc, flags: Cardinal;
  var time: Cardinal): integer;
begin
  Result := canReadSpecific(FCanHnd, id, @msg, dlc, flags, time);
end;

function TCanChannelEx.ReadSync(timeout: Cardinal): integer;
begin
  Result := canReadSync(FCanHnd, timeout);
end;

function TCanChannelEx.ReadSyncSpecific(id: Longint; timeout: Cardinal): integer;
begin
  Result := canReadSyncSpecific(FCanHnd, id, timeout);
end;

procedure TCanChannelEx.SetNotifyFlags(aValue: TCanChanNotifyFlags);
var
  result: Integer;
begin
  FNotifyFlags := aValue;
  if (not (csDesigning in ComponentState)) and FBusActive then begin
    AssertActive;
    result := canSetNotify(FCanHnd, FWinHandle, N2I(FNotifyFlags));
    Check(result, 'SetNotifyFlags failed');
  end;
end;

procedure TCanChannelEx.SetWinHandle(aValue: HWND);
var
  result: Integer;
begin
  FWinHandle := aValue;
  if (not (csDesigning in ComponentState)) and FBusActive then begin
    AssertActive;
    result := canSetNotify(FCanHnd, FWinHandle, N2I(FNotifyFlags));
    Check(result, 'SetWinHandle failed');
  end;
end;

procedure TCanChannelEx.SetBusActive(aValue: Boolean);
var
  result: Integer;
begin
  if (csDesigning in ComponentState) or
    (FBusActive = aValue) or
    (not Active) then Exit;

  if FDllFailed then raise ECanChan.Create('Could not find the CAN driver.');

  if aValue then begin
    UpdateCANCircuitParameters;
    if (FNotifyFlags <> []) then begin
      result := canSetNotify(FCanHnd, FWinHandle, N2I(FNotifyFlags));
      Check(result, 'setNotify failed');
    end;
    result := canBusOn(FCanHnd);
    Check(result, 'Could not change state to Bus On');
  end else begin
    result := canBusOff(FCanHnd);
    Check(result, 'Could not change state to Bus Off');
  end;
  FBusActive := aValue;
  if Assigned(FOnStateChanged) then FOnStateChanged(Self);
end;

procedure TCanChannelEx.SetOnCan(Index: Integer; aProc: TNotifyEvent);
var n: TCanChanNotifyFlags;
begin
  case Index of
    1: FOnCanRx := aProc;
    2: FOnCanTx := aProc;
    3: FOnStateChanged := aProc;
  end;
  n := FNotifyFlags;
  if not Assigned(aProc) then begin
    case Index of
      1: Exclude(n, ccNotifyRx);
      2: Exclude(n, ccNotifyTx);
      3: Exclude(n, ccNotifyStatus);
    end;
  end else begin
    case Index of
      1: Include(n, ccNotifyRx);
      2: Include(n, ccNotifyTx);
      3: Include(n, ccNotifyStatus);
    end;
  end;
  SetNotifyFlags(n);
end;

function TCanChannelEx.GetOnCan(Index: Integer): TNotifyEvent;
begin
  case Index of
    1: result := FOnCanRx;
    2: result := FOnCanTx;
    3: Result := FOnStateChanged;
  else
    result := nil;
  end;
end;

function TCanChannelEx.GetChannelStatus: Cardinal;
var stat: canStatus;
begin
  stat := canReadStatus(FCanHnd, Result);
  Check(stat, 'Read CAN status failed');
end;

function TCanChannelEx.GetNow: Cardinal;
begin
  Result := canReadTimer(FCanHnd);
end;

function TCanChannelEx.GetTransmitErrors: Cardinal;
var tx, rx, ov: Cardinal;
  stat: integer;
begin
  stat := canReadErrorCounters(FCanHnd, tx, rx, ov);
  Check(stat, 'canReadErrorCounters');
  Result := tx;
end;

function TCanChannelEx.GetReceiveErrors: Cardinal;
var tx, rx, ov: Cardinal;
  stat: integer;
begin
  stat := canReadErrorCounters(FCanHnd, tx, rx, ov);
  Check(stat, 'canReadErrorCounters');
  Result := rx;
end;

function TCanChannelEx.GetOverruns: Cardinal;
var tx, rx, ov: Cardinal;
  stat: integer;
begin
  stat := canReadErrorCounters(FCanHnd, tx, rx, ov);
  Check(stat, 'canReadErrorCounters');
  Result := ov;
end;

function TCanChannelEx.GetChannelCount: Cardinal;
var tmp: Integer;
begin
  canGetNumberOfChannels(tmp);
  Result := tmp;
end;

function TCanChannelEx.WaitForEvent(timeout: Cardinal): Integer;
begin
  Result := canWaitForEvent(FCanHnd, timeout);
end;

function TCanChannelEx.SetHardwareFilters(id: Longint; flag: Cardinal): integer;
begin
  Result := canAccept(FCanHnd, id, flag);
end;

function TCanChannelEx.ReadSpecificSkip(id: Longint; var msg; var dlc, flags: Cardinal; var time: Cardinal): Integer;
begin
  Result := canReadSpecificSkip(FCanHnd, id, @msg, dlc, flags, time);
end;

function TCanChannelEx.GetChannelNames(idx: Integer): string;
var stat: Integer;
  p: AnsiString;
begin
  SetLength(Result, 64);
    SetLength(p, 64);
  stat := canGetChannelData(idx, canCHANNELDATA_CHANNEL_NAME, p[1], Length(Result));
  Result := string(p);//PChar(p); // without using ansistring and string this was producing garbage
  Check(stat, 'GetChannelNames');
end;

function TCanCHannelEx.GetChannelName: String;
var stat: Integer;
begin
  SetLength(Result, 64);
  stat := canGetChannelData(Channel, canCHANNELDATA_CHANNEL_NAME, Result[1], Length(Result));
  Result := PChar(Result);
  Check(stat, 'GetChannelName');
end;



end.

