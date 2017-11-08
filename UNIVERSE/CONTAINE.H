#if !defined(__OWL_CONTAINER_H)
#define __OWL_CONTAINER_H

#define INCL_PM

#include <os2.h>
#include <control.h>
#include <gdiobjec.h>

// EIGENE FELDER FšR DETAIL-VIEW
typedef struct _RECORDEXT
{
  ULONG         ulAve;
  ULONG         ulMax;
  HICON         hico;
} RECORDEXT, *PRECORDEXT;

// RECORDCORE ERWEITERN
typedef struct _RECORDCNR
{
  RECORDCORE    RecordCore;
  RECORDEXT     RecordExt;
} RECORDCNR, *PRECORDCNR;



class _OWLCLASS TContainer;

class _OWLCLASS TContainer : public TControl
      {
         protected:     CNRINFO                 CI;
                        FIELDINFO              *pLastFI;
                        FIELDINFOINSERT         FII;
                        RECORDCNR              *pLastRC;
                        RECORDINSERT            RI;
                        LONG                    lSortMode;
                        LONG                    lFilterMode;

         public:        TContainer (TWindow *parent, int resourceId, TModule *module = 0)
                       :TControl (parent, resourceId, module) {};
                        TContainer (TWindow *parent, TResId id, int x, int y, int w, int h,
                                    TModule *module = 0)
                       :TControl (parent,id,NULL,x,y,w,h,module)
                          { Attr.Style |= CCS_AUTOPOSITION | CCS_SINGLESEL; }
                       // šBERSCHREIBEN FšR EXTRA-SETUP
                        virtual void    SetupWindow();
                       // CONTAINER TITLE & ANSICHTEN
                        BOOL            SetTitle (PSZ pszTitle = NULL, char chAlign = 'C');
                        BOOL            SetIconView (BOOL fMini = FALSE, BOOL fTree = FALSE);
                        BOOL            SetTextView (BOOL fFlow = FALSE, BOOL fTree = FALSE);
                        BOOL            SetNameView (BOOL fMini = FALSE, BOOL fFlow = FALSE);
                        BOOL            SetDetailView (BOOL fTitles = FALSE);
                       // AUSWAHLM™GLICHKEITEN (EINFACH, ERWEITERT, MEHRFACH)
                        void            SetSingleSelection ();
                        void            SetExtendedSelection ();
                        void            SetMultipleSelection ();
                       // RECORDS SORTIEREN
                        virtual SHORT   CompareRecord (RECORDCNR *pRC1, RECORDCNR *pRC2,
                                                       void *pStorage = NULL);
                        void            SetSortMode (LONG lSM = 0) { lSortMode = lSM; };
                        LONG            GetSortMode() { return (lSortMode); };
                        BOOL            Sort (void *pStorage = NULL);
                       // RECORDS FILTERN
                        virtual BOOL    FilterRecord (RECORDCNR *pRC, void *pStorage = NULL)
                                          { return TRUE; };
                        void            SetFilterMode (LONG lFM = 0) { lFilterMode = lFM; };
                        LONG            GetFilterMode() { return (lFilterMode); };
                        BOOL            Filter (void *pStorage = NULL);
                       // RECORD SUCHEN
                        RECORDCNR      *SearchRecord (PSZ pszText, RECORDCNR *pRC = NULL,
                                                      BOOL fPrefix = TRUE, BOOL fCase = FALSE);
                       // EINGABEFELD ™FFNEN/SCHLIESSEN
                        BOOL            CloseEdit();
                        BOOL            OpenEdit (RECORDCNR *pRC, FIELDINFO *pFI = NULL);
                       // MARKIERUNG DEFINIEREN
                        ULONG           SetEmphasis (RECORDCNR *pRC, BOOL fChange = TRUE,
                                                     BOOL fSelected = TRUE,
                                                     BOOL fCursored = FALSE,
                                                     BOOL fInUse = FALSE);
                        RECORDCNR      *GetEmphasis (RECORDCNR *pRC, BOOL fSelected = TRUE,
                                                     BOOL fCursored = FALSE,
                                                     BOOL fInUse = FALSE);
                       // NUR ICON-VIEW
                        BOOL            ArrangeIcons ();
                       // NUR TREE-VIEW
                        BOOL            CollapseTree (RECORDCNR *pRC = NULL);
                        BOOL            ExpandTree (RECORDCNR *pRC = NULL);
                        BOOL            SetTreeIndent (LONG lTI = -1);
                        BOOL            SetTreeLine (LONG lTL = -1, BOOL fDraw = TRUE);
                        BOOL            SetTreeIcons (TIcon *pIcoExp, TIcon *pIcoCol);
                       // NUR DETAIL-VIEW
                        BOOL            ScrollSplitWindow (BOOL fLeft, LONG lPixel);
                        BOOL            SetSplitBar (LONG lSB = -1);
                        BOOL            SetLastSplitColumn (FIELDINFO *pFI = NULL);
                       // FšR ALLE RECORDS
                        void            SetReadOnly (BOOL fRO = TRUE);
                        void            SetAutoPosition (BOOL fAuto = TRUE);
                        void            SetPointerVerification (BOOL fVerify = FALSE);
                        BOOL            ScrollWindow (BOOL fDir, LONG lPixel);
                        BOOL            SetLineSpacing (LONG lLS = -1);
                       // LETZTE EINGEFšGTE POINTER ERMITTELN
                        RECORDCNR      *GetLastRecord() { return (pLastRC); };
                        FIELDINFO      *GetLastDetail() { return (pLastFI); };
                       // NORMALEN RECORD EINFšGEN
                        ULONG           InsertRecord (RECORDCNR  *pRClevel,
                                                      RECORDCNR  *pRCpred,
                                                      RECORDEXT  *pRE,
                                                      PSZ         pszText,
                                                      TPoint     *pIconPos,
                                                      TIcon      *pStdIcon,
                                                      TIcon      *pMiniIcon,
                                                      BOOL        fHide = FALSE,
                                                      BOOL        fDrop = FALSE,
                                                      BOOL        fReadOnly = FALSE);
                       // DETAIL-BESCHREIBUNG EINFšGEN
                        ULONG           InsertDetail (FIELDINFO  *pFIpred,
                                                      PSZ         pszTitle,
                                                      ULONG       ulOfs = FIELDOFFSET(RECORDCNR,RecordCore.pszText),
                                                      char        chType = 'S',
                                                      ULONG       ulColWidth = 100,
                                                      char        chAlignData = 'L',
                                                      char        chAlignTitle = 'C',
                                                      BOOL        fReadOnly = TRUE);
                       // RECORD & DETAIL L™SCHEN
                        LONG            RemoveRecord (RECORDCNR *pRC = NULL);
                        LONG            RemoveDetail (FIELDINFO *pFI = NULL);
                       // SCROLLBARS ZUM LEBEN ERWECKEN
                        void            EvHScroll (UINT scrollCode, UINT thumbPos, HWND hWndCtl);
                        void            EvVScroll (UINT scrollCode, UINT thumbPos, HWND hWndCtl);
                       // PROTECTED
         protected:     void            SetStyle (ULONG ulStyle);
                        ULONG           ClearStyle (ULONG ulStyle);
                        BOOL            SetView (ULONG ulFlags);
                        char far       *GetClassName();
                       // RESPONSE TABLE
                        DECLARE_RESPONSE_TABLE(TContainer);
                        DECLARE_STREAMABLE(_OWLCLASS, TContainer, 1);
      };

#endif

