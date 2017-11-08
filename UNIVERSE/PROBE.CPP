
#include <containe.h>
#include <applicat.h>
#include <dialog.h>
#include <framewin.h>
#include <dc.h>
#include <gdiobjec.h>

#include <string.h>



// MENU & COMMANDS
#define IDM_PROBE              1
#define IDCM_DEMOCONTAINER    101

// MAIN DIALOGS
#define IDDLG_CONTAINER        2

#define IDCT_CONTAINER       101

#define IDICO_FULL             1
#define IDICO_MINI             2



#define IDRB_VIEWICONS                  201
#define IDRB_VIEWMINIICONS              202
#define IDRB_VIEWICONTREE               203

#define IDRB_VIEWTEXT                   211
#define IDRB_VIEWTEXTFLOWED             212
#define IDRB_VIEWTEXTTREE               213

#define IDRB_VIEWNAMES                  221
#define IDRB_VIEWMININAMES              222
#define IDRB_VIEWNAMESFLOWED            223
#define IDRB_VIEWMININAMESFLOWED        224

#define IDRB_VIEWDETAILS                231
#define IDRB_VIEWDETAILSTITLES          232

#define IDCB_CONTAINERTITLE             401

#define IDRB_SELSINGLE                  301
#define IDRB_SELEXTENDED                302
#define IDRB_SELMULTIPLE                303

#define IDB_SORTAZ                      801
#define IDB_SORTZA                      802
#define IDB_ANYTEST                     803

#define IDB_TREESAMPLE                  807
#define IDB_ICONSAMPLE                  808
#define IDB_DETAILSAMPLE                809




class TDlgContainer : public TDialog
     {
        public:        TContainer   *pIContainer;
                       RECORDCNR   *pRCpred;
                       RECORDCNR   *pRClevel;
                       FIELDINFO    *pFI;
                       char         *pszFonts;
                       ULONG         dwOfs,dwIdx;
                       ULONG         ulAve[1024];
                       ULONG         ulMax[1024];
                       HICON         hico[1024];
                       TIcon        *pBig;
                       TIcon        *pMini;
                       TIcon        *pPlus;
                       TIcon        *pMinus;

        public:        TDlgContainer (TWindow *parent, TResId resID, TModule *module = 0)
                      :TDialog (parent, resID, module) {}

        protected:     BOOL CanClose();
                       void SetupWindow();
                      // VIEWS
                       void RBViewIcons() { pIContainer->SetIconView(); };
                       void RBViewMiniIcons() { pIContainer->SetIconView(TRUE); };
                       void RBViewIconTree() { pIContainer->SetIconView(FALSE,TRUE); }
                       void RBViewText() { pIContainer->SetTextView(); }
                       void RBViewTextFlowed() { pIContainer->SetTextView(TRUE); };
                       void RBViewTextTree() { pIContainer->SetTextView(FALSE,TRUE); };
                       void RBViewNames() { pIContainer->SetNameView(); };
                       void RBViewMiniNames() { pIContainer->SetNameView(TRUE); };
                       void RBViewNamesFlowed() { pIContainer->SetNameView(FALSE,TRUE); };
                       void RBViewMiniNamesFlowed() { pIContainer->SetNameView(TRUE,TRUE); };
                       void RBViewDetails() { pIContainer->SetDetailView(); };
                       void RBViewDetailsTitles() { pIContainer->SetDetailView(TRUE); };
                      // MISC
                       void CBContainerTitle();
                      // SELECTION
                       void RBSelSingle() { pIContainer->SetSingleSelection(); };
                       void RBSelExtended() { pIContainer->SetExtendedSelection(); };
                       void RBSelMultiple() { pIContainer->SetMultipleSelection(); };
                      // SORT
                       void BtnSortAZ() { pIContainer->SetSortMode(1); pIContainer->Sort(); }
                       void BtnSortZA() { pIContainer->SetSortMode(-1); pIContainer->Sort(); }
                       void BtnAnyTest();
                      // SAMPLE
                       void BtnTreeSample();
                       void BtnIconSample();
                       void BtnDetailSample();
                      // DECLARE RESPONSE TABLE
                       DECLARE_RESPONSE_TABLE(TDlgContainer);
     };



DEFINE_RESPONSE_TABLE1(TDlgContainer,TDialog)
  EV_COMMAND (IDRB_VIEWICONS,RBViewIcons),
  EV_COMMAND (IDRB_VIEWMINIICONS,RBViewMiniIcons),
  EV_COMMAND (IDRB_VIEWICONTREE,RBViewIconTree),
  EV_COMMAND (IDRB_VIEWTEXT,RBViewText),
  EV_COMMAND (IDRB_VIEWTEXTFLOWED,RBViewTextFlowed),
  EV_COMMAND (IDRB_VIEWTEXTTREE,RBViewTextTree),
  EV_COMMAND (IDRB_VIEWNAMES,RBViewNames),
  EV_COMMAND (IDRB_VIEWMININAMES,RBViewMiniNames),
  EV_COMMAND (IDRB_VIEWNAMESFLOWED,RBViewNamesFlowed),
  EV_COMMAND (IDRB_VIEWMININAMESFLOWED,RBViewMiniNamesFlowed),
  EV_COMMAND (IDRB_VIEWDETAILS,RBViewDetails),
  EV_COMMAND (IDRB_VIEWDETAILSTITLES,RBViewDetailsTitles),
  EV_COMMAND (IDRB_SELSINGLE,RBSelSingle),
  EV_COMMAND (IDRB_SELEXTENDED,RBSelExtended),
  EV_COMMAND (IDRB_SELMULTIPLE,RBSelMultiple),
  EV_COMMAND (IDB_SORTAZ,BtnSortAZ),
  EV_COMMAND (IDB_SORTZA,BtnSortZA),
  EV_COMMAND (IDCB_CONTAINERTITLE,CBContainerTitle),
  EV_COMMAND (IDB_ANYTEST,BtnAnyTest),
  EV_COMMAND (IDB_TREESAMPLE,BtnTreeSample),
  EV_COMMAND (IDB_ICONSAMPLE,BtnIconSample),
  EV_COMMAND (IDB_DETAILSAMPLE,BtnDetailSample),
END_RESPONSE_TABLE;




void TDlgContainer::CBContainerTitle()
{
  IsDlgButtonChecked(IDCB_CONTAINERTITLE)
    ?  pIContainer->SetTitle ("Some Fonts...")
    :  pIContainer->SetTitle();
}

BOOL TDlgContainer::CanClose()
{
  pIContainer->RemoveRecord();
  pIContainer->RemoveDetail();
  delete pIContainer;
  free (pszFonts);
  delete pBig;
  delete pMini;
  delete pPlus;
  delete pMinus;
 // CLOSE
  return (TRUE);
}



char  chFirst;
BOOL  fWithExt;


static int CALLBACK EnumFontsProc (LOGFONT *lplf, TEXTMETRIC *tm, ULONG, LPARAM lpData)
{
  TDlgContainer        *pDlg;
  int                   iLen;
  RECORDEXT             RE;
  RECORDEXT            *pRE;

  if (lplf->lfFaceName[0] != chFirst) return (1);
  pDlg = (TDlgContainer*)lpData;
 // FONTNAME BUFFER
  iLen = strlen(lplf->lfFaceName);
  memcpy (&pDlg->pszFonts[pDlg->dwOfs],lplf->lfFaceName,iLen);
 // ERWEITERUNGS-RECORD KLARMACHEN
  pRE = NULL;
  if (fWithExt)
    {
      pDlg->ulAve[pDlg->dwIdx] = tm->tmAveCharWidth;
      pDlg->ulMax[pDlg->dwIdx] = tm->tmMaxCharWidth;
      pDlg->hico[pDlg->dwIdx] = HICON(*pDlg->pBig);
      RE.ulAve = pDlg->ulAve[pDlg->dwIdx];
      RE.ulMax = pDlg->ulMax[pDlg->dwIdx];
      RE.hico = pDlg->hico[pDlg->dwIdx];
      pDlg->dwIdx++;
      pRE = &RE;
    }
 // RECORD EINFšGEN
  pDlg->pIContainer->InsertRecord (pDlg->pRClevel,      // ebene oder NULL fr erste ebene
                                   pDlg->pRCpred,       // vorg„nger-record
                                   pRE,                 // zeiger auf erweiterung oder NULL
                                   &pDlg->pszFonts[pDlg->dwOfs],  // fontname - icon text
                                   &TPoint(0,0),        // egal, da AUTOPOSITION !
                                   pDlg->pBig,          // groáer standard-icon
                                   pDlg->pMini);        // kleiner mini-icon
 // HINTER DEN LETZTEN EINGEFšGTEN RECORD SOLL AUCH GLEICH DER NŽCHSTE EINGEFšGT WERDEN
  pDlg->pRCpred = pDlg->pIContainer->GetLastRecord();
 // UPDATE OFFSET FOR NAME BUFFER
  pDlg->dwOfs += (iLen+1);
 // RETURN
  return 1;
}




void TDlgContainer::BtnIconSample()
{
  TScreenDC     dc;

  memset (pszFonts,0x00,32000);
  dwOfs = 0;
  pIContainer->RemoveRecord();
  pIContainer->RemoveDetail();
  pIContainer->SetAutoPosition(TRUE);
 // RECORDS EINFšGEN. DAS GEHT RECHT EINFACH, DA ALLE
 // ICONS AUF DER ROOT-EBENE EINGEFšGT WERDEN. MAN
 // BEN™TIGT DESWEGEN AUCH KEINEN PARENT.
  pRCpred = NULL;                             // beginn signalisieren, "1ter" record
  pRClevel = NULL;                            // erste ebene, also NULL
//  chFirst = 'F';                              // alle fonts mit einem 'F' einfgen
  fWithExt = FALSE;                           // keine zus„tzlichen details
  dc.EnumFonts (0,(OLDFONTENUMPROC)EnumFontsProc,this);
 // DAMIT WIR IN DER DETAIL-VIEW AUCH WAS SEHEN K™NNEN,
 // LEGEN WIR DIE ERSTE UND EINZIGE SPALTE FEST. HIER
 // WERDEN DANN UNTEREINANDER ALLE FONTNAMEN ANGEZEIGT.
  pIContainer->InsertDetail (NULL,"F...-Fonts");
}


void TDlgContainer::BtnTreeSample()
{
  TScreenDC     dc;

  memset (pszFonts,0x00,32000);
  dwOfs = 0;
  pIContainer->RemoveRecord();
  pIContainer->RemoveDetail();
  pIContainer->SetTreeIndent(30);
  pIContainer->SetTreeLine(1,TRUE);
  pIContainer->SetTreeIcons (pMinus,pPlus);
 // BEGINN DES ERSTEN ASTES
  pIContainer->InsertRecord( NULL,            // erste ebene
                             NULL,            // kein vorg„nger
                             NULL,            // erweiterung nicht notwendig
                             "F-Fonts",       // icon text, also ast-text
                             &TPoint(0,0),    // egal, da autoposition
                             pBig,            // groáer standard-icon
                             pMini);          // kleiner mini-icon
 // UND JETZT DIE FONTNAMEN AN DEN AST HŽNGEN
  pRCpred = NULL;                             // beginn signalisieren, "1ter" record
  pRClevel = pIContainer->GetLastRecord();    // eingefgter record ist n„chste ebene !
//  chFirst = 'F';                              // fonts mit einem 'F' einfgen
  fWithExt = FALSE;                           // keine zus„tzlichen details
  dc.EnumFonts (0,(OLDFONTENUMPROC)EnumFontsProc,this);

 // BEGINN DES ZWEITEN ASTES
  pIContainer->InsertRecord (NULL,                      // erste ebene
                             pRClevel,                  // vorg„nger auf der ersten ebene !!!
                             NULL,                      // erweiterung nicht notwendig
                             "S-Fonts",                 // icon text, also ast-text
                             &TPoint(0,0),              // egal
                             pBig,                      // groáer standard-icon
                             pMini);                    // kleiner mini-icon
 // UND JETZT DIE FONTNAMEN AN DEN AST HŽNGEN
  pRCpred = NULL;                              // beginn signalisieren, "1ter" record
  pRClevel = pIContainer->GetLastRecord();     // eingefgter record ist n„chste ebene !
  chFirst = 'S';                               // fonts mit einem 'S' einfgen
  fWithExt = FALSE;                            // keine zus„tzlichen details
  dc.EnumFonts (0,(OLDFONTENUMPROC)EnumFontsProc,this);
 // AST-NAMEN IN DER DETAIL-VIEW ANZEIGEN.
  pIContainer->InsertDetail (NULL,"Žste");
}


void TDlgContainer::BtnDetailSample()
{
  TScreenDC     dc;

  memset (pszFonts,0x00,32000);
  dwOfs = 0;
  pIContainer->RemoveRecord();
  pIContainer->RemoveDetail();
 // RECORDS EINFšGEN WIE BEIM ICON-SAMPLE.
 // JETZT WIRD ABER ZUSŽTZLICH DER ERWEITERUNGS-RECORD
 // GEFšLLT, DAMIT DIE DETAIL-VIEW AUCH WAS HERGIBT.
 // IN ALLEN ANDEREN VIEWS WERDEN DIESE ZUSŽTZLICHEN
 // INFORMATIONEN LOGISCHERWEISE NICHT ANGEZEIGT.
  pRCpred = NULL;                             // beginn signalisieren, "1ter" record
  pRClevel = NULL;                            // erste ebene
  chFirst = 'F';                              // alle fonts mit einem 'F' einfgen
  fWithExt = TRUE; dwIdx = 0;                 // zu„tzliche details fr detail-view
  dc.EnumFonts (0,(OLDFONTENUMPROC)EnumFontsProc,this);
 // IN DER DETAIL-VIEW WIRD ZUSŽTZLICH AUF DIE FELDER DES
 // ERWEITERUNGS-RECORDS ZUGEGRIFFEN. DIE ERSTE SPALTE
 // GIBT - WIE IMMER - DIE FONTNAMEN AN. DIE PARAMETER FšR
 // DIE ERSTE SPALTE SIND EIGENTLICH šBERFLšSSIG, DA SIE DEN
 // STANDARD DARSTELLEN. ABER FšR'S VERSTŽNDNIS SEIEN SIE HIER
 // NOCH MAL ANGEGEBEN.
  pIContainer->InsertDetail (NULL,"F...-Fonts",FIELDOFFSET(RECORDCNR,RecordCore.pszText),'S',200);
  pFI = pIContainer->GetLastDetail();
  pIContainer->SetLastSplitColumn(pFI);
  pIContainer->InsertDetail (pFI,"AveCharWidth",FIELDOFFSET(RECORDCNR,RecordExt.ulAve),'U',100,'R');
  pFI = pIContainer->GetLastDetail();
  pIContainer->InsertDetail (pFI,"MaxCharWidth",FIELDOFFSET(RECORDCNR,RecordExt.ulMax),'U',100,'R');
  pFI = pIContainer->GetLastDetail();
  pIContainer->InsertDetail (pFI,"Ein Icon",FIELDOFFSET(RECORDCNR,RecordExt.hico),'B');
 // SPIELEN MIT DEM SPLIT WINDOW
  pIContainer->SetSplitBar(50);
}


void TDlgContainer::SetupWindow()
{
  TDialog::SetupWindow();
 // MEMORY FOR FONT NAMES
  pszFonts = (char *)malloc(32000);
 // ICONS
  pBig = new TIcon (GetModule()->GetInstance(),IDICO_FULL);
  pMini = new TIcon (GetModule()->GetInstance(),IDICO_MINI);
  pPlus = new TIcon (GetModule()->GetInstance(),11);
  pMinus = new TIcon (GetModule()->GetInstance(),12);
 // CREATE CONTAINER-OBJECT
  pIContainer = new TContainer(this,IDCT_CONTAINER);
  pIContainer->Create();
 // SET DEFAULT VIEW & SELECTION
  pIContainer->SetIconView();
  CheckDlgButton (IDRB_VIEWICONS,1);
  pIContainer->SetSingleSelection();
  CheckDlgButton (IDRB_SELSINGLE,1);
}



void TDlgContainer::BtnAnyTest()
{
  pIContainer->ScrollSplitWindow (TRUE,100);
}





class TWinProbe : public TFrameWindow
     {
        public:     TDlgContainer *pIDlgContainer;

        public:          TWinProbe (TWindow *parent, const char far *title = 0);
        protected:  void CmDemoContainer ();
                    DECLARE_RESPONSE_TABLE(TWinProbe);
     };

DEFINE_RESPONSE_TABLE1(TWinProbe,TFrameWindow)
  EV_COMMAND (IDCM_DEMOCONTAINER,CmDemoContainer),
END_RESPONSE_TABLE;

void TWinProbe::CmDemoContainer()
{
  pIDlgContainer = new TDlgContainer (this,IDDLG_CONTAINER);
  pIDlgContainer->Execute();
  delete pIDlgContainer;
}


TWinProbe::TWinProbe (TWindow *parent, const char far *title)
          :TFrameWindow (parent, title, 0, FALSE, 0)
{
  Init (parent,FALSE);
}

class TAppProbe : public TApplication
     {
        public:          TAppProbe() : TApplication() {}
                    void InitMainWindow();
     };

void TAppProbe::InitMainWindow()
{
  SetMainWindow (new TWinProbe(0, "c't - Container Demo"));
  GetMainWindow()->AssignMenu (IDM_PROBE);
}

int OwlMain(int /*argc*/, char* /*argv*/ [])
{
  return TAppProbe().Run();
}

