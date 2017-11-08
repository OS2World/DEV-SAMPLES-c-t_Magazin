USES dos;
CONST
  ttl		 ='*** Super ***, ver 0.02 17.03.95';
  ttl2		 ='  <c> -mat ù All rights reserved ù';
  debug 	 :Boolean=False;

type
  lp = ^longint;
  wp = ^word;

VAR
  q,p            :ARRAY[0..520]OF Byte;   {reQuest and rePly buffers}
  bbfr           :ARRAY[0..127]OF Byte;              {bindery buffer}

  is_sv, can_switch, can_sw2	    :Boolean;
  nv_cmd, nv_ver, nv_ver2 :Byte;
  maxuser,ix              :Word;
  my_id_hl,l1    :LongInt;
  myself         :String;

CONST
  prop_seceq	 ='SECURITY_EQUALS';
  prop_osv       ='OBJ_SUPERVISORS';
  sv             ='SUPERVISOR';

  {--- auxilliary routines ----}
  FUNCTION b2h(b:Byte):String;            {- byte to hex}
  CONST hs       :ARRAY[0..15]OF Char='0123456789ABCDEF';
  BEGIN
    b2h[0]:=#2; b2h[1]:=hs[b SHR 4]; b2h[2]:=hs[b AND $f];
  END;

  FUNCTION l2h(l:LongInt):String;    {- longint to hex}
  VAR la         :ARRAY[0..1]OF Word ABSOLUTE l;
  BEGIN
    l2h:=b2h(Hi(la[1]))+b2h(Lo(la[1]))+
         b2h(Hi(la[0]))+b2h(Lo(la[0]));
  END;

  FUNCTION PStr(VAR p)         :String;
    {- ASCIIZ to PASCAL string}
  VAR
    s :String; i :Byte;
  BEGIN
    Move(p,s[1],255);
    i:=1;
    WHILE(i<255)AND(s[i]<>#0)DO Inc(i);
    s[0]:=Char(i-1);
    PStr:=s;
  END;

  FUNCTION lswap(l:LongInt):LongInt;	 {- motorola to intel format}

  INLINE( $5A/$58/				     {pop dx, pop ax}
    $86/$E0/                                          {xchg    ah,al}
    $86/$F2);                                         {xchg    dh,dl}

  FUNCTION WrN(b:Boolean):String;
  BEGIN IF b THEN WrN:='' ELSE WrN:='not '; END;

  {---- aux novell ----}

  FUNCTION NvLogicalID:Byte;		 {- return connection number}

  INLINE($B4/$DC/                                         {MOV AH,DC}
         $CD/$21);                                           {INT 21}

  PROCEDURE ReqInit(cmd:Byte;siz:Word);
    {- init request and reply buffers}
    {- init sizes, enter request cmd byte, enter reply packet size}

  BEGIN
    FillChar(q,SizeOf(q),0);                  { clear request buffer}
    FillChar(p,SizeOf(p),0);                    { clear reply buffer}
    q[0]:=1;	   q[1]:=0;			       {command only}
    q[2]:=cmd;	   nv_cmd:=cmd; 			{set command}
    p[0]:=Lo(siz); p[1]:=Hi(siz);	       {size of reply buffer}
  END;

  PROCEDURE ReqByte(b:Byte);		 {- add a byte to req buffer}
  VAR pp         : ^Word;
  BEGIN
    pp:= @q[0];                                     {get length word}
    q[pp^+2]:=b;
    Inc(pp^);
  END;

  PROCEDURE ReqWord(w:Word);		 {- add a word to req buffer}
  VAR pp         : ^Word;
  BEGIN
    pp:= @q[0];                                     {get length word}
    q[pp^+2]:=Lo(w); q[pp^+3]:=Hi(w);
    Inc(pp^,2);
  END;

  PROCEDURE ReqLong(l:LongInt);       {- add a longint to req buffer}
  VAR pp         : ^Word;
  BEGIN
    pp:= @q[0];                                     {get length word}
    Move(l,q[pp^+2],4);
    Inc(pp^,4);
  END;

  PROCEDURE ReqStr(s:String);	{- add a pascal string to req buffer}

  VAR i          :Word;
  BEGIN
    FOR i:=0 TO Length(s) DO ReqByte(Byte(s[i]));
  END;

  PROCEDURE NE(nv_err:Byte);	{- print network error and terminate}
  BEGIN
    IF nv_err<>0 THEN BEGIN
        WriteLn(' network Error!! request = ',b2h(nv_cmd),
                ' err = ',b2h(nv_err));
        Halt($ff);
      END;
  END;

  FUNCTION NetCall:Byte;    { q and p are reQuest and rePly packets }
  VAR r          :registers;
  BEGIN
    r.ds:=Seg(q);r.si:=Ofs(q);
    r.es:=Seg(p);r.di:=Ofs(p);
    r.ax:=$e300;
    r.bx:=0;r.cx:=0;r.dx:=0;                     {for safety reasons}
    msdos(r);
    NetCall:=r.al;
  END;

  PROCEDURE Wr(description:String);	      {- write debug message}
  BEGIN
    ReqInit($36,$36);                       {get bindery object name}
    ReqLong(l1);
    NE(NetCall);
    Write(description,' for object ',myself,':   ');
    WriteLn(l2h(lswap(l1)),' ',b2h(p[6]),' ',PStr(p[8]));
  END;

  FUNCTION RdProp(user,prop:String):Byte;
  BEGIN  {-read property value}
    ReqInit($3d,$82);
    ReqWord($100);                                             {user}
    ReqStr(user);
    ReqByte(1);        {current segment, increment for further calls}
    ReqStr(prop);
    RdProp:=NetCall;
    Move(p,bbfr,128);                           {save bindery buffer}
  END;

  FUNCTION GetSecurity : byte;
  BEGIN  {- scan property value -> read security byte}
    ReqInit($3c,$18);
    ReqWord($100);
    ReqStr(myself);
    ReqLong(-1);
    ReqStr(prop_seceq);
    NE(netcall);
    GetSecurity:=p[$13];				 { <- result}
  END;

  PROCEDURE security(b:Byte);   {### 4 ###}
  BEGIN
    ReqInit($3b,0);
    ReqWord($100);
    ReqStr(myself);
    ReqByte(b); 				 {new security value}
    ReqStr(prop_seceq);
    NE(netcall);
  END;

  FUNCTION ChkSupEq:Boolean;
  BEGIN 		       {- check if I'm supervisor equivalent}
    ChkSupEq:=False;				    {preset variable}
    {read property "SECURITY_EQUALS"}
    IF RdProp(myself,prop_seceq)<>0 THEN Exit;           {read error}
    ix:=2;					     {start @ byte 2}
    REPEAT
      Move(bbfr[ix],l1,4);Inc(ix,4);  {get equivalence ID - end if 0}
      IF l1=0 THEN Break;	               {break on end of list}
      IF debug THEN Wr(prop_seceq);
      IF(lswap(l1)=1)THEN ChkSupEq:=True;              {found SV-ID?}
      IF(PStr(p[8])=sv)THEN ChkSupEq:=True;              {or SV name}
    UNTIL ix>128;                                {until buffer empty}
  END;

  FUNCTION ChkMan(user:String):Boolean;
  BEGIN 			  {- check if I am manager of "user"}
    ChkMan:=False;
    IF RdProp(user,prop_osv)<>0 THEN Exit;	 {exit on read error}
    ix:=2;
    REPEAT
      Move(bbfr[ix],l1,4);Inc(ix,4);  {get equivalence ID - end if 0}
      IF l1=0 THEN Break;
      IF debug THEN Wr(prop_osv);
      IF(l1=my_id_hl)                              {found my own ID?}
      THEN BEGIN ChkMan:=True; Break; END;         {then I'm manager}
    UNTIL ix>128;                                {until buffer empty}
  END;

  FUNCTION ChkManagers:Boolean;
  BEGIN 	  {- check if current account is supervisor manager}
    ChkManagers:=ChkMan(myself)AND ChkMan(sv);
  END;


  PROCEDURE Switch_SV(b:Boolean);
    {- switches supervisor equivalence}
    {- check current state / security before calling netware}
  BEGIN  {don't switch on if already on, don't switch off if alr off}
    IF is_sv=b THEN Exit;	 {drop this and get a netware error!}

    IF b
    THEN ReqInit($41,0)                {-- add bindery object to set}
    ELSE ReqInit($42,0);          {-- delete bindery object from set}
    {except this, buffers are identical for both functions}
    ReqWord($100);			     {type user - big endian}
    ReqStr(myself);                                       {user name}
    ReqStr(prop_seceq);                             {SECURITY_EQUALS}
    ReqWord($100);        {1 = user / big endian! type of SUPERVISOR}
    ReqStr(sv);                                          {SUPERVISOR}
    NE(NetCall);
    is_sv:=b;
  END;

  PROCEDURE ClrMan(user:String);
  BEGIN 		{- clears myself as object manager of "user"}
    {delete managers}
    ReqInit($42,0);               {-- delete bindery object from set}
    ReqWord($100);                           {type user - big endian}
    ReqStr(user);                                         {user name}
    ReqStr(prop_osv);                               {OBJ_SUPERVISORS}
    ReqWord($100);
    ReqStr(myself);                                       {user name}
    NE(NetCall);

    NE(RdProp(user,prop_osv));
    ix:=2;
    REPEAT
      Move(bbfr[ix],l1,4);Inc(ix,4);  {get equivalence ID - end if 0}
      IF l1=0 THEN Break;
    UNTIL ix>128;
    IF ix=6                                    {if property is empty}
    THEN BEGIN
        ReqInit($3A,0);                             {delete property}
        ReqWord($100);
        ReqStr(user);                                     {user name}
        ReqStr(prop_osv);                           {OBJ_SUPERVISORS}
        NE(NetCall);
      END;
  END;

  PROCEDURE SetMan(user:String);
  BEGIN 	      {- installs myself as object manager of "user"}
    IF RdProp(user,prop_osv)<>0                {does property exist?}
    THEN 
      BEGIN					     {no - create it}
        {create property}
        ReqInit($39,0);
        ReqWord($100);                                         {user}
        ReqStr(user);                                     {user name}
        ReqByte(2);                                             {SET}
	ReqByte($32);		    {### -1- ### try to change that!}
        ReqStr(prop_osv);                           {OBJ_SUPERVISORS}
        NE(NetCall);
      END;

    {add bindery object to set}
    ReqInit($41,0);                    {-- add bindery object to set}
    ReqWord($100);
    ReqStr(user);                                         {user name}
    ReqStr(prop_osv);                               {OBJ_SUPERVISORS}
    ReqWord($100);
    ReqStr(myself);                                          {myself}
    NE(NetCall);
  END;

  PROCEDURE SwitchSwitch(foo:Boolean);
  BEGIN
    IF foo
    THEN BEGIN				    {switch switchability on}
        IF NOT ChkSupEq THEN Halt(2);          {must be SV eq here!!}
        SetMan(myself);                    {add myself as my manager}
        SetMan(sv);                        {add myself as SV manager}
	can_switch:=True;		 {my account is prepared now}
      END
    ELSE 
      BEGIN				   {switch switchability off}
        ClrMan(myself);                  {clear myself as my manager}
        ClrMan(sv);                      {clear myself as SV manager}
        can_switch:=False;
      END;
  END;


BEGIN  {- main}
  WriteLn(ttl+ttl2);
  IF ParamStr(1)='/debug' THEN debug:=True;       {write info or not}
  IF ParamStr(2)='/debug' THEN debug:=True;

  IF NvLogicalID=0 THEN
    BEGIN WriteLn(#7'Novell not active!');Halt($ff); END;

  ReqInit($11,202);
  NE(Netcall);
  nv_ver := p[$32]; nv_ver2 := p[$33];     {get novell version}
  maxuser:=swap(wp(@p[$34])^);
  IF (nv_ver)<>3 THEN BEGIN writeln('need Novell 3.x'); halt($ff); END;
  {writeln('Novell ',nv_ver,'.',nv_ver2,' ',maxuser,' users.');{}

  {--- get object information}
  ReqInit($16,$3e);
  ReqByte(NvLogicalID);
  NE(NetCall);

  my_id_hl:=lp(@p[2])^; 			 {store not swapped!}
  myself:=PStr(p[8]);

  IF lswap(my_id_hl)=1 THEN 
    BEGIN	    {am I "the" SUPERVISOR ?}
      WriteLn('You ARE the SUPERVISOR! - terminating...');
      Halt(1);					  {.. nothing to do!}
    END;

  is_sv:=ChkSupEq;
  can_switch:=ChkManagers;
  can_sw2 := (GetSecurity SHR 4) <= 2;

  IF NOT can_sw2 THEN                    {skip that if SECURITY type}
  IF is_sv AND NOT can_switch
  THEN SwitchSwitch(True);	       {### -2- ###  make switchable}

  IF can_switch OR	      {am I my own and supervisor's manager?}
     can_sw2                 {can I write SECURITY_EQUALS directly ?}
    THEN
    BEGIN				       {yes, I can switch SV}
      IF(ParamStr(1)='-')THEN Switch_SV(False);
      IF(ParamStr(1)='+')THEN Switch_SV(True);
      IF(ParamStr(1)='­') and not can_sw2 THEN
        BEGIN
          Switch_SV(True);SwitchSwitch(False);Switch_SV(False);
        END;
    END;

  Write(myself,' is ',WrN(is_sv),'supervisor equivalent ');
  Write('(',WrN(can_switch or can_sw2),'switchable)');
  IF can_sw2 THEN Write(' <security byte>');
  WriteLn;
  if debug then begin
    ChkSupEq;
    ChkManagers;
  end;

  Halt(Byte(is_sv));

END.
