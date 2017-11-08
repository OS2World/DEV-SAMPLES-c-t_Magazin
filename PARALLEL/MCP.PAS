{ MacPrint Centronics-Printer-Interface fÅr Macintosh und andere     }
{ tcycle=1.0851 us bei 11,059 MHz Takt, 2764 EPROM                   }
{ Da nur rund 60 Bytes RAM zur VerfÅgung stehen, wurde konsequent    }
{ auf lokale Variable verzichtet. Der verwendete KSC-Pascal-Compiler }
{ Version 4.19 legt leider auch lokale Variable dauerhaft und nicht  }
{ auf dem Stack ab.                                                  }
{ P3.4 Baudrate, verbunden mit: }
{ --   1200                     }
{ P1.0 2400   (J3)              }
{ P1.1 4800   (J4)              }
{ P1.2 9600   (J5)              }
{ P1.3 19200  (J6)              }
{ P1.4 57600  (J7)              }
{ P3.7 Protokoll  (J1)          }
{ 0    XOn/XOff Software + CTS  }
{ 1    nur CTS Hardware         }
{ CTS wird immer bedient, egal, ob Soft- oder Hardware-Handshake.    }
{ Das Programm benutzt einen maxBuf gro·en Ringpuffer zur Vermeidung }
{ von allzu gro·em Handshake-Overhead, besonders bei XOn/XOff-Prot.  }
{ Auf Centronics-Seite wird nur BUSY, nicht jedoch /ACK ausgewertet. }

program macprint;

  const
    VersStr = 'c''t-MacPrint (c) C.Meyer 4/95 V1.01 ';
    msgStr1 = 'ABCDEFGHIJKLMNOPQRSTUVWXYZéôö ';
    msgStr2 = 'abcdefghijklmnopqrstuvwxyzÑîÅ ';
    msgStr3 = '0123456789!"$%&/()=[]*+#,.-  ';
    high    = true;
    low     = false;
    HskCTS  = 2;      { Pins Port P3, Hsk/CTS Ausgang }
    Strobe  = 3;      { Strobe zum Printer }
    BdSel   = 4;      { Bits/s Sel  Jumper Common Pin }
    Busy    = 5;      { high = Printer Busy }
    AuxSel  = 6;
    HardHsk = 7;      { high = Hardware-Handshake }

    maxBuf  = 31;     { Buffer-Grî·e }
    fullBuf = 27;     { etwas Reserve zum Buffer-Ende }
    XOn     = 17;     { Software-Handshake-Konstanten }
    XOff    = 19;
    Bd12    = 208;
    Bd24    = 232;    { Timer-Konstanten fÅr Baudrate }
    Bd48    = 244;
    Bd96    = 250;
    Bd192   = 253;
    Bd576   = 255;

  type
    bufType = packed array[0..maxbuf] of byte;

  var
    Iptr,Lptr: byte;
    dataBuf: BufType; { Ringpuffer fÅr eingehende Daten }
    x: byte;
    countInt:Integer;
    XOnSent: Boolean;

{************************* Interrupt-Routinen ****************************}

  procedure interrupt serial;
{ Char in SBUF angekommen oder komplett gesendet }
  begin
    if RI then
      begin
        dataBuf[Iptr]:=SBUF;
        RI:=false;
        inc(Iptr);
        if Iptr > maxBuf then
          Iptr:=0;
{ bevor Buffer Åberfahren wird: feststellen, ob Grenzwert erreicht }
        if abs(IPtr-LPtr) > fullBuf then
          begin
            P3.HskCTS:=high;   { Einen Moment...    CTS-Handshake sperren }
            if XOnSent and (not P3.HardHsk) then    { XOn/XOff-Enable-Pin }
              begin            { XOff senden, sofern noch nicht geschehen }
                TI := false;
                SBUF:=XOff;
                XOnSent:=false;
              end;
          end;
      end;
    if TI then
{ da XOn/XOff relativ selten gesendet wird, mu· nicht auf erfolgte }
{ öbertragung gewartet werden. Gesetztes TI landet irgendwann hier }
      TI:=false;
  end;

{************************* Low-Level-Routinen ****************************}

  procedure wait(zehntelsec:integer);
{ Zehntelsekunden abgestimmt auf 11,059 MHz Takt }
  begin
    for countInt:= 0 to zehntelsec*43 do
      for x:=0 to 253 do
  end;

  procedure rwait;
{ FÅr Timing: Luxus-NOP, incl. Aufruf 10 us bei 11,059 MHz }
  begin
    x:=x;
    x:=x;
  end;


{************************* allgemeine Routinen ***************************}

  procedure ParOut(theByte:byte);
{ Byte in Centronics-Manier ausgeben, 10us /Strobe }
  begin
    P1:=theByte;
    rwait;
    P3.Strobe:=low;
    rwait;
    P3.Strobe:=high;
    rwait;
    repeat
    until not P3.Busy;  { Busy abwarten }
    rwait;              { Ack  abwarten: 30us maximal }
    rwait;
    rwait;
    P1:=$FF;
  end;

  procedure message;
  begin
    repeat
      for x:= 1 to 128 do
        ParOut(byte(VersStr[x]));
    until P3.AuxSel;
  end;

{*************************** Hauptschleife *******************************}

begin
  reset(serial);
  IE:=0;
  PS:=true;              { Serial hîchste PrioritÑt  }
  ES:=true;              { Serial input Int enable   }
  EA:=true;              { Interrupt enable          }
  P3:=$FF;
  P3.HskCTS:=low;        { HskIn Mac freigeben       }
  RI:=false;             { Interrupt-Flag lîschen    }
  for x:=0 to maxBuf do  { Buffer initialisieren     }
    dataBuf[x]:=0;
  XOnSent:=false;        { XOn noch nicht gesendet   }
  x:=$FE;
  Iptr:=0;
  for Lptr := 1 to 5 do
    begin
{ alle Bit/s Jumper abfragen }
      P1:=x;
      if not P3.BdSel then Iptr := Lptr;
      x:= 1 or (x shl 1);
    end;
  case Iptr of
    0: TH1:=Bd12;        { 1200  bit/s, 1mal blinken (kein Jumper) }
    1: TH1:=Bd24;        { 2400  bit/s, 2mal blinken (J3) }
    2: TH1:=Bd48;        { 4800  bit/s, 3mal blinken (J4) }
    3: TH1:=Bd96;        { 9600  bit/s, 4mal blinken (J5) }
    4: TH1:=Bd192;       { 19200 bit/s, 5mal blinken (J6) }
    5: TH1:=Bd576;       { 57600 bit/s, 6mal blinken (J7) }
  end;
  for Lptr := 0 to Iptr do
    begin                { Blinken zur Funktionskontrolle }
      P1:=$55;
      wait(2);
      P1:=$AA;
      wait(2);
    end;
  P1:=$FF;
  Iptr:=0;               { Interrupt-Pointer und }
  Lptr:=0;               { Loop-Pointer auf Daten-Array }

  repeat
    if not P3.AuxSel then
      message;
    if IPtr<>LPtr then   { Buffer abgearbeitet? }
      begin
        ParOut(dataBuf[Lptr]);
        inc(Lptr);
        if Lptr > MaxBuf then
          LPtr:=0;       { immer im Kreis }
      end
    else
{ CTS-Freigabe-Impuls und XOn fÅr Handshake }
      begin
        P3.HskCTS:=low;  { weiter gehts: CTS freigeben }
        if (not XOnSent) and (not P3.HardHsk) then
          begin          { XOn senden, sofern noch nicht geschehen }
            TI:=false;
            SBUF:=XOn;
            XOnSent := true;
          end;
        P1:=$FE;         { LED dimmen (PWM) }
        P1:=$FF;
      end;
  until false;
end.
