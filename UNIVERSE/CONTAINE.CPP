
#include <containe.h>

// KLEINES HELFERLEIN...
TContainer *pICNR;

DEFINE_RESPONSE_TABLE1 (TContainer,TControl)
  EV_WM_HSCROLL,
  EV_WM_VSCROLL,
END_RESPONSE_TABLE;


void TContainer::EvHScroll (UINT scrollCode, UINT thumbPos, HWND hWndCtl)
{
  EvCommand (GetId(),hWndCtl,MAKELONG(thumbPos,scrollCode));
}

void TContainer::EvVScroll (UINT scrollCode, UINT thumbPos, HWND hWndCtl)
{
  EvCommand (GetId(),hWndCtl,MAKELONG(thumbPos,scrollCode));
}

static BOOL CALLBACK pfnFilterRecord (RECORDCNR *pRC, void *pStorage)
{
  return (pICNR->FilterRecord(pRC,pStorage));
}

BOOL TContainer::Filter (void *pStorage)
{
  pICNR = this;
  return (BOOL)WinSendMsg(HWindow,CM_FILTER,pfnFilterRecord,pStorage);
}

SHORT TContainer::CompareRecord (RECORDCNR *pRC1, RECORDCNR *pRC2, void *pStorage)
{
  return (strcmp(pRC1->RecordCore.pszText,pRC2->RecordCore.pszText) * GetSortMode());
}

static SHORT CALLBACK pfnCompareRecord (RECORDCNR *pRC1, RECORDCNR *pRC2, void *pStorage)
{
  return (pICNR->CompareRecord(pRC1,pRC2,pStorage));
}

BOOL TContainer::Sort (void *pStorage)
{
  pICNR = this;
  return (BOOL)WinSendMsg (HWindow,CM_SORTRECORD,pfnCompareRecord,pStorage);
}

BOOL TContainer::CloseEdit()
{
  return (BOOL)WinSendMsg (HWindow,CM_CLOSEEDIT,NULL,NULL);
}

BOOL TContainer::OpenEdit (RECORDCNR *pRC, FIELDINFO *pFI)
{
  CNREDITDATA   CED;

 // SETUP CNREDITDATA STRUCTURE
  CED.cb = sizeof(CNREDITDATA);         // struktur-grî·e
  CED.hwndCnr = HWindow;                // fenster-handle
  CED.pRecord = (RECORDCORE *)pRC;      // recordcore
  CED.pFieldInfo = pFI;                 // fieldinfo
  CED.ppszText = NULL;                  // immer NULL bei CM_OPENEDIT
  CED.cbText = 0;                       // immer 0 bei CM_OPENEDIT
  CED.id = GetId();                     // ID, spez. Bereiche nicht unterstÅtzt (READONLY !)
 // OPEN EDIT
  return (BOOL)WinSendMsg (HWindow,CM_OPENEDIT,&CED,NULL);
}

void TContainer::SetReadOnly (BOOL fRO)
{
  SetStyle (ClearStyle(CCS_READONLY) | (fRO ? CCS_READONLY : 0));
}

void TContainer::SetAutoPosition (BOOL fAuto)
{
  SetStyle (ClearStyle(CCS_AUTOPOSITION) | (fAuto ? CCS_AUTOPOSITION : 0));
}

void TContainer::SetPointerVerification (BOOL fVerify)
{
  SetStyle (ClearStyle(CCS_VERIFYPOINTERS) | (fVerify ? CCS_VERIFYPOINTERS : 0));
}

void TContainer::SetSingleSelection ()
{
  SetStyle (ClearStyle(CCS_SINGLESEL | CCS_EXTENDSEL | CCS_MULTIPLESEL) | CCS_SINGLESEL);
}

void TContainer::SetExtendedSelection ()
{
  SetStyle (ClearStyle(CCS_SINGLESEL | CCS_EXTENDSEL | CCS_MULTIPLESEL) | CCS_EXTENDSEL);
}

void TContainer::SetMultipleSelection ()
{
  SetStyle (ClearStyle(CCS_SINGLESEL | CCS_EXTENDSEL | CCS_MULTIPLESEL) | CCS_MULTIPLESEL);
}

ULONG TContainer::SetEmphasis (RECORDCNR *pRC, BOOL fChange, BOOL fSelected,
                               BOOL fCursored, BOOL fInUse)
{
  USHORT usAttr;

  if (pRC == NULL) return(-1);
  usAttr = (fSelected ? CRA_SELECTED : 0) | (fCursored ? CRA_CURSORED : 0) |
           (fInUse ? CRA_INUSE : 0);
  return (ULONG)WinSendMsg (HWindow,CM_SETRECORDEMPHASIS,pRC,MPFROM2SHORT(fChange,usAttr));
}

RECORDCNR *TContainer::GetEmphasis (RECORDCNR *pRC, BOOL fSelected, BOOL fCursored, BOOL fInUse)
{
  USHORT usAttr;

  if (pRC == NULL) pRC = (RECORDCNR *)CMA_FIRST;
  usAttr = (fSelected ? CRA_SELECTED : 0) | (fCursored ? CRA_CURSORED : 0) |
           (fInUse ? CRA_INUSE : 0);
  return (RECORDCNR *)WinSendMsg (HWindow,CM_QUERYRECORDEMPHASIS,pRC,MPFROMSHORT(usAttr));
}

RECORDCNR *TContainer::SearchRecord (PSZ pszText, RECORDCNR *pRC, BOOL fPrefix, BOOL fCase)
{
  SEARCHSTRING          SS;

  if (pRC == NULL) pRC = (RECORDCNR *)CMA_FIRST;      // begin search after "pRC"
 // SETUP STRUCTURE
  SS.cb = sizeof(SEARCHSTRING);                         // size of this structure
  SS.pszSearch = pszText;                               // search text
  SS.usView = CV_TEXT;                                  // don't care, all the same
  SS.fsPrefix = (ULONG)fPrefix;                         // leading | substring
  SS.fsCaseSensitive = (ULONG)fCase;                    // case | not case sensitive
 // SEARCH
  return (RECORDCNR *)WinSendMsg (HWindow,CM_SEARCHSTRING,&SS,pRC);
}

ULONG TContainer::InsertRecord (RECORDCNR *pRClevel, RECORDCNR *pRCpred,
                                RECORDEXT *pRE, PSZ pszText,
                                TPoint *pIconPos,
                                TIcon *pStdIcon, TIcon *pMiniIcon,
                                BOOL fHide, BOOL fDrop, BOOL fReadOnly)
{
  ULONG    ul;

 // EINIGE HILFSVARIABLEN SETZEN
  pLastRC = NULL; pICNR = this;
 // ALLOC MEMORY FOR ONE RECORD, ADDITIONAL MEMORY !
  ul = sizeof (RECORDEXT);
  pLastRC = (RECORDCNR *)WinSendMsg (HWindow,CM_ALLOCRECORD,MPFROMLONG(ul),MPFROMSHORT(1));
  if (pLastRC == NULL) return (-1);
 // SETUP RECORDCNR STRUCTURE
  ul = (fHide ? CRA_FILTERED : 0);                                  // nicht anzeigen
  ul |= (fDrop ? CRA_DROPONABLE : 0);                               // direkt manipulierbar
  ul |= (fReadOnly ? CRA_RECORDREADONLY : 0);                       // nicht editierbar
  pLastRC->RecordCore.cb = sizeof(RECORDCORE);                      // size of structure
  pLastRC->RecordCore.flRecordAttr = ul;                            // attributes
  pLastRC->RecordCore.preccNextRecord = NULL;                       // no next record
  pLastRC->RecordCore.ptlIcon.x = pIconPos->x;                      // icon-view: x-pos
  pLastRC->RecordCore.ptlIcon.y = pIconPos->y;                      // icon-view: y-pos
  pLastRC->RecordCore.hptrIcon = (HPOINTER)HICON(*pStdIcon);        // standard icon-handle
  pLastRC->RecordCore.hptrMiniIcon = (HPOINTER)HICON(*pMiniIcon);   // mini icon-handle
  pLastRC->RecordCore.hbmBitmap = 0;                                // nicht unterstÅtzt
  pLastRC->RecordCore.hbmMiniBitmap = 0;                            // nicht unterstÅtzt
  pLastRC->RecordCore.pTreeItemDesc = NULL;                         // nicht unterstÅtzt
  pLastRC->RecordCore.pszText = pLastRC->RecordCore.pszIcon =              // gleicher text
  pLastRC->RecordCore.pszName = pLastRC->RecordCore.pszTree = pszText;     // fÅr alle views
 // ERWEITERUNGS-RECORD FöR DETAILS KOPIEREN
  if (pRE) memmove (&pLastRC->RecordExt,pRE,sizeof(RECORDEXT));
 // SETUP RECORDINSERT STRUCTURE
  if (pRCpred == NULL) pRCpred = (RECORDCNR *)CMA_FIRST;        // 1ter record, kein vorgÑnger
  RI.cb = sizeof(RECORDINSERT);                                 // struktur-grî·e
  RI.pRecordOrder = (RECORDCORE *)pRCpred;                      // vorgÑnger-record
  RI.pRecordParent = (RECORDCORE *)pRClevel;                    // ebene
  RI.fInvalidateRecord = TRUE;                                  // auto. update display
  RI.zOrder = CMA_TOP;                                          // at top of z-order
  RI.cRecordsInsert = 1;                                        // 1 record einfÅgen
 // INSERT RECORD
  ul = (ULONG)WinSendMsg (HWindow,CM_INSERTRECORD,(PRECORDCORE)pLastRC,&RI);
  if (ul == 0) return (-2);
 // PASSED
  return (0);
}

LONG TContainer::RemoveRecord (RECORDCNR *pRC)
{
  USHORT usCnt = 1;

 // ALLE RECORDS LôSCHEN
  if (pRC == NULL) usCnt = 0;
 // LôSCHEN
  return (LONG)WinSendMsg (HWindow,CM_REMOVERECORD,pRC,MPFROM2SHORT(usCnt,CMA_FREE | CMA_INVALIDATE));
}

ULONG TContainer::InsertDetail (FIELDINFO *pFIpred, PSZ pszTitle,
                                ULONG ulOfs, char chType, ULONG ulColWidth,
                                char chAlignData, char chAlignTitle, BOOL fReadOnly)
{
  ULONG   ulFlagsData,ulFlagsTitle;
  ULONG   ulAlignData,ulAlignTitle;
  ULONG   ul;

 // EINIGE HILFSVARIABLEN SETZEN
  pLastFI = NULL; pICNR = this;
 // SPEICHER FöR DETAIL-RECORD ANFORDERN
  pLastFI = (FIELDINFO *)WinSendMsg (HWindow,CM_ALLOCDETAILFIELDINFO,MPFROMLONG(1),NULL);
  if (pLastFI == NULL) return (-1);
 // AUSRICHTUNGEN KLARMACHEN, TEXT & TITEL
  ulAlignData = ulAlignTitle = CFA_CENTER;                              // zentriert
  if (chAlignData == 'L') ulAlignData = CFA_LEFT;                       // linksbÅndig
  if (chAlignData == 'R') ulAlignData = CFA_RIGHT;                      // rechtsbÅndig
  if (chAlignTitle == 'L') ulAlignTitle = CFA_LEFT;                     // linksbÅndig
  if (chAlignTitle == 'R') ulAlignTitle = CFA_RIGHT;                    // rechtsbÅndig
 // FLAGS FöR DIE DATEN IN DEN SPALTEN
  ulFlagsData = CFA_VCENTER;                              // daten vertikal zentrieren
  ulFlagsData |= CFA_SEPARATOR;                           // vertikale trennlinie
  ulFlagsData |= CFA_HORZSEPARATOR;                       // trennlinie unter spaltentitel
  ulFlagsData |= (fReadOnly ? CFA_FIREADONLY : 0);        // daten editieren ?
  ulFlagsData |= (chType == 'B' ? CFA_BITMAPORICON : 0);  // data ist bitmap/icon
  ulFlagsData |= (chType == 'S' ? CFA_STRING : 0);        // data ist string
  ulFlagsData |= (chType == 'U' ? CFA_ULONG : 0);         // data ist ulong
  ulFlagsData |= (chType == 'D' ? CFA_DATE : 0);          // data ist date
  ulFlagsData |= (chType == 'T' ? CFA_TIME : 0);          // data ist time
 // FLAGS FöR DEN SPALTENTITEL
  ulFlagsTitle = CFA_VCENTER;                           // titel vertikal zentrieren
  ulFlagsTitle |= CFA_FITITLEREADONLY;                  // title kann nicht editiert werden
 // SETUP FIELDINFO STRUCTURE
  pLastFI->cb = sizeof(FIELDINFO);                      // struktur-grî·e
  pLastFI->flData = ulFlagsData | ulAlignData;          // daten-atribute
  pLastFI->flTitle = ulFlagsTitle | ulAlignTitle;       // titel-attribute
  pLastFI->pTitleData = pszTitle;                       // spaltentitel
  pLastFI->offStruct = ulOfs;                           // offset zu den daten
  pLastFI->pUserData = NULL;                            // keine user-daten
  pLastFI->pNextFieldInfo = NULL;                       // nur diese spalte
  pLastFI->cxWidth = ulColWidth;                        // spalten-breite
 // SETUP FIELDINFOINSERT STRUCTURE
  if (pFIpred == NULL) pFIpred = (FIELDINFO *)CMA_FIRST;  // erster record
  FII.cb = sizeof(FIELDINFOINSERT);                     // struktur-grî·e
  FII.pFieldInfoOrder = pFIpred;                        // spalten-reihenfolge oder NULL (ende)
  FII.fInvalidateFieldInfo = TRUE;                      // anzeige updaten
  FII.cFieldInfoInsert = 1;                             // 1 record einfÅgen
 // INSERT DETAIL RECORD
  ul = (ULONG)WinSendMsg (HWindow,CM_INSERTDETAILFIELDINFO,pLastFI,&FII);
  if (ul == 0) return (-1);
 // PASSED
  return (0);
}

LONG TContainer::RemoveDetail (FIELDINFO *pFI)
{
  USHORT usCnt = 1;

 // ALLE RECORDS LôSCHEN
  if (pFI == NULL) usCnt = 0;
 // LôSCHEN
  return (LONG)WinSendMsg (HWindow,CM_REMOVEDETAILFIELDINFO,pFI,MPFROM2SHORT(usCnt,CMA_FREE | CMA_INVALIDATE));
}

BOOL TContainer::SetDetailView (BOOL fTitles)
{
  ULONG   ulCV;

 // SET FLAGS & VIEW
  ulCV = CV_DETAIL;
  ulCV |= (fTitles ? CA_DETAILSVIEWTITLES : 0);
  return SetView (ulCV);
}

BOOL TContainer::ScrollSplitWindow (BOOL fLeft, LONG lPixel)
{
  return (BOOL)WinSendMsg (HWindow,CM_HORZSCROLLSPLITWINDOW,
                           MPFROMSHORT((fLeft ? CMA_LEFT : CMA_RIGHT)),MPFROMLONG(lPixel));
}

BOOL TContainer::SetSplitBar (LONG lSB)
{
 // SETUP CNRINFO STRUCTURE
  CI.cb = sizeof(CNRINFO);                      // struktur-grî·e
  CI.xVertSplitbar = lSB;                       // balken-position
  return (BOOL)WinSendMsg (HWindow,CM_SETCNRINFO,&CI,MPFROMLONG(CMA_XVERTSPLITBAR));
}

BOOL TContainer::SetLastSplitColumn (FIELDINFO *pFI)
{
 // SETUP CNRINFO STRUCTURE
  CI.cb = sizeof(CNRINFO);                      // struktur-grî·e
  CI.pFieldInfoLast = pFI;                      // neue spalte oder NULL
  return (BOOL)WinSendMsg (HWindow,CM_SETCNRINFO,&CI,MPFROMLONG(CMA_PFIELDINFOLAST));
}

BOOL TContainer::SetTextView (BOOL fFlow, BOOL fTree)
{
  ULONG   ulCV;

 // CONTAINER CANNOT DISPLAY FLOWED TREE...
  if (fTree) fFlow = FALSE;
 // SET FLAGS & VIEW
  ulCV = CV_TEXT;
  ulCV |= (fFlow ? CV_FLOW : 0);
  ulCV |= (fTree ? CV_TREE : 0);
  return SetView (ulCV);
}

BOOL TContainer::SetIconView (BOOL fMini, BOOL fTree)
{
  ULONG   ulCV;

 // CONTAINER CANNOT DISPLAY TREE AND MINI-ICONS
  if (fTree) fMini = FALSE;
 // SET FLAGS & VIEW
  ulCV = CV_ICON;
  ulCV |= (fMini ? CV_MINI : 0);
  ulCV |= (fTree ? CV_TREE : 0);
  return SetView (ulCV);
}

BOOL TContainer::ArrangeIcons ()
{
  return (BOOL)WinSendMsg (HWindow,CM_ARRANGE,NULL,NULL);
}

BOOL TContainer::SetNameView (BOOL fMini, BOOL fFlow)
{
  ULONG   ulCV;

 // SET FLAGS & VIEW
  ulCV = CV_NAME;
  ulCV |= (fMini ? CV_MINI : 0);
  ulCV |= (fFlow ? CV_FLOW : 0);
  return SetView (ulCV);
}

BOOL TContainer::CollapseTree (RECORDCNR *pRC)
{
  if (pRC == NULL) return (FALSE);
  return (BOOL)WinSendMsg (HWindow,CM_COLLAPSETREE,pRC,NULL);
}

BOOL TContainer::ExpandTree (RECORDCNR *pRC)
{
  if (pRC == NULL) return (FALSE);
  return (BOOL)WinSendMsg (HWindow,CM_EXPANDTREE,pRC,NULL);
}

BOOL TContainer::SetTreeIndent (LONG lTI)
{
 // SETUP CNRINFO STRUCTURE
  CI.cb = sizeof(CNRINFO);                      // struktur grî·e
  CI.cxTreeIndent = lTI;                        // neuer indent-wert
  return (BOOL)WinSendMsg (HWindow,CM_SETCNRINFO,&CI,MPFROMLONG(CMA_CXTREEINDENT));
}

BOOL TContainer::SetTreeLine (LONG lTL, BOOL fDraw)
{
  ULONG         ul;

 // GET CURRENT CONTAINER INFO & CLEAR ALL DRAW FLAG
  WinSendMsg (HWindow,CM_QUERYCNRINFO,&CI,MPFROMSHORT(sizeof(CNRINFO)));
  ul = CI.flWindowAttr & (0xFFFFFFFF - (CA_TREELINE));
 // UPDATES DRAW FLAG
  if (fDraw) ul |= CA_TREELINE;                 // zeichne linie
 // SETUP CNRINFO STRUCTURE
  CI.cb = sizeof(CNRINFO);                      // struktur-grî·e
  CI.cxTreeLine = lTL;                          // neue strichstÑrke
  CI.flWindowAttr = ul;                         // zeichenflag !
  return (BOOL)WinSendMsg (HWindow,CM_SETCNRINFO,&CI,MPFROMLONG(CMA_FLWINDOWATTR | CMA_CXTREELINE));
}

BOOL TContainer::SetTreeIcons (TIcon *pIcoExp, TIcon *pIcoCol)
{
 // SETUP CNRINFO STRUCTURE
  CI.cb = sizeof(CNRINFO);                              // structure size
  CI.hptrExpanded = (HPOINTER)HICON(*pIcoExp);          // expanded icon
  CI.hptrCollapsed = (HPOINTER)HICON(*pIcoCol);         // collapsed icon
 // SET ICONS
  return (BOOL)WinSendMsg (HWindow,CM_SETCNRINFO,&CI,MPFROMLONG(CMA_TREEICON));
}

BOOL TContainer::SetView (ULONG ulFlags)
{
  ULONG ul;

 // GET CURRENT CONTAINER INFO & CLEAR ALL VIEW FLAGS
  WinSendMsg (HWindow,CM_QUERYCNRINFO,&CI,MPFROMSHORT(sizeof(CNRINFO)));
  ul = CI.flWindowAttr & (0xFFFFFFFF - (CV_ICON | CV_NAME | CV_TEXT | CV_TREE |
                                        CV_MINI | CV_FLOW |
                                        CV_DETAIL | CA_DETAILSVIEWTITLES));
 // SETUP CNRINFO STRUCTURE
  CI.cb = sizeof(CNRINFO);              // structure size
  CI.flWindowAttr = ul | ulFlags;       // view flags
  return (BOOL)WinSendMsg (HWindow,CM_SETCNRINFO,&CI,MPFROMLONG(CMA_FLWINDOWATTR));
}

BOOL TContainer::SetLineSpacing (LONG lLS)
{
 // SETUP CNRINFO STRUCTURE
  CI.cb = sizeof(CNRINFO);                      // structure size
  CI.cyLineSpacing = lLS;                       // new value
  return (BOOL)WinSendMsg (HWindow,CM_SETCNRINFO,&CI,MPFROMLONG(CMA_LINESPACING));
}

BOOL TContainer::SetTitle (PSZ pszTitle, char chAlign)
{
  ULONG         ul;

 // GET CURRENT CONTAINER INFO & CLEAR ALL TITLE FLAGS
  WinSendMsg (HWindow,CM_QUERYCNRINFO,&CI,MPFROMSHORT(sizeof(CNRINFO)));
  ul = CI.flWindowAttr & (0xFFFFFFFF - (CA_CONTAINERTITLE | CA_TITLEREADONLY |
                                        CA_TITLESEPARATOR | CA_TITLECENTER |
                                        CA_TITLELEFT | CA_TITLERIGHT));
 // UPDATES TITLE FLAGS
  ul |= CA_CONTAINERTITLE | CA_TITLEREADONLY;   // title ist read-only
  ul |= (pszTitle ? CA_TITLESEPARATOR : 0);     // trennlinie, wenn text
  if (chAlign == 'L') ul |= CA_TITLELEFT;       // linksbÅndig
  if (chAlign == 'C') ul |= CA_TITLECENTER;     // zentriert
  if (chAlign == 'R') ul |= CA_TITLERIGHT;      // rechtsbÅndig
 // SETUP CNRINFO STRUCTURE
  CI.cb = sizeof(CNRINFO);                      // struktur-grî·e
  CI.pszCnrTitle = pszTitle;                    // title oder NULL
  CI.flWindowAttr = ul;                         // flags
 // SETZT TITEL
  return (BOOL)WinSendMsg (HWindow,CM_SETCNRINFO,&CI,MPFROMLONG(CMA_FLWINDOWATTR | CMA_CNRTITLE));
}

BOOL TContainer::ScrollWindow (BOOL fDir, LONG lPixel)
{
  USHORT  usDir;

  usDir = (fDir ? CMA_VERTICAL : CMA_HORIZONTAL);
  return (BOOL)WinSendMsg (HWindow,CM_SCROLLWINDOW,MPFROMSHORT(usDir),MPFROMLONG(lPixel));
}

void TContainer::SetupWindow()
{
 // SETUPWINDOW VON TCONTROL AUFRUFEN
  TControl::SetupWindow();
 // EINIGE VARIABLEN INTIALISIEREN
  pLastRC = NULL; pLastFI = NULL;
  lFilterMode = 0;
 // SORTIEREN INITIALISIEREN
  lSortMode = 0; pICNR = this;                          // nicht sortieren
  CI.cb = sizeof(CNRINFO);                              // struktur grî·e
  CI.pSortRecord = pfnCompareRecord;                    // sortier-funktion
 // "CompareRecord" AUFRUFEN BEIM EINFöGEN NEUER RECORDS
  WinSendMsg (HWindow,CM_SETCNRINFO,&CI,MPFROMLONG(CMA_PSORTRECORD));
}

void TContainer::SetStyle (ULONG ulStyle)
{
  SetWindowLong (QWL_STYLE,ulStyle);
  Invalidate ();
}

ULONG TContainer::ClearStyle (ULONG ulStyle)
{
  return (GetWindowLong(GWL_STYLE) & (0xFFFFFFFF - ulStyle));
}

char far *TContainer::GetClassName()
{
  return (WC_CONTAINER);
}

