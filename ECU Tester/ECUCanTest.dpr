program WinCanTest;

uses
  Vcl.Forms,
  CanTest in 'CanTest.pas' {MainForm},
  CanChanEx in 'canchanex.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.CreateForm(TMainForm, MainForm);
  Application.Run;
end.
