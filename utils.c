/*****************************************************************************
 ** LVMPM - utils.c                                                         **
 *****************************************************************************
 * This file contains various general-purpose utility routines.              *
 *****************************************************************************/
#include "lvmpm.h"

/* ------------------------------------------------------------------------- *
 * CentreWindow                                                              *
 *                                                                           *
 * Centres one window relative to another (or to the screen).  The window    *
 * will always be placed on top of the z-order (HWND_TOP).                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *     HWND  hwndCentre  : the window to be centred                          *
 *     HWND  hwndRelative: the window relative to which hwndCentre will be   *
 *                         centred, or NULLHANDLE to centre on the screen    *
 *     ULONG flFlags     : additional flags for WinSetWindowPos (SWP_MOVE is *
 *                         always assumed                                    *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void CentreWindow( HWND hwndCentre, HWND hwndRelative, ULONG flFlags )
{
    LONG x, y,       // x & y coordinates of hwndCentre (relative to hwndRelative)
         ox, oy,     // x & y offsets (i.e. coordinates of hwndRelative)
         rcx, rcy;   // width & height of hwndRelative
    SWP  wp;         // window-position structure


    if ( !hwndRelative ) {
        rcx = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
        rcy = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
        ox = 0;
        oy = 0;
    }
    else {
        if ( ! WinQueryWindowPos( hwndRelative, &wp )) return;
        rcx = wp.cx;
        rcy = wp.cy;
        ox = wp.x;
        oy = wp.y;
    }
    if ( WinQueryWindowPos( hwndCentre, &wp )) {
        x = ( rcx / 2 ) - ( wp.cx / 2 );
        y = ( rcy / 2 ) - ( wp.cy / 2 );
        WinSetWindowPos( hwndCentre, HWND_TOP,
                         x + ox, y + oy, wp.cx, wp.cy, SWP_MOVE | flFlags );
    }

}


/* ------------------------------------------------------------------------- *
 * CheckDBCS()                                                               *
 *                                                                           *
 * Employ a somewhat simplistic check to determine if we're running on a     *
 * DBCS version of the OS.                                                   *
 *                                                                           *
 * ARGUMENTS: N/A                                                            *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if ?:\OS2DBCS exists                                               *
 *   FALSE if ?:\OS2DBCS does not exist, or if its existence could not be    *
 *     determined                                                            *
 * ------------------------------------------------------------------------- */
BOOL CheckDBCS( void )
{
    APIRET      rc;
    ULONG       ulBootVol;
    CHAR        chBootVol,
                szDFile[ 12 ];

    rc = DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                          &ulBootVol, sizeof(ULONG)      );
    if ( rc != NO_ERROR ) return FALSE;

    chBootVol = 64 + ulBootVol;
    sprintf( szDFile, "%c:\\OS2DBCS", chBootVol );

    return FileExists( szDFile );
}


/* ------------------------------------------------------------------------- *
 * FileExists()                                                              *
 *                                                                           *
 * Check to see if the specified file exists.                                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PSZ pszFile: Name of the file to look for                               *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if the file exists                                                 *
 *   FALSE if the file does not exist, or if its existence could not be      *
 *     determined                                                            *
 * ------------------------------------------------------------------------- */
BOOL FileExists( PSZ pszFile )
{
    FILESTATUS3 fsts3 = {0};
    APIRET      rc;

    rc = DosQueryPathInfo( pszFile, FIL_STANDARD, &fsts3, sizeof( fsts3 ));
    if ( rc == NO_ERROR ) return TRUE;

    return FALSE;
}


/* ------------------------------------------------------------------------- *
 * GetLanguageFile()                                                         *
 *                                                                           *
 * Generates what should be the correct, language-specific, pathname of the  *
 * specied language-dependent file.  Note that this function does NOT        *
 * determine whether or not the file actually exists; that is up to the      *
 * caller.                                                                   *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   PSZ pszFullPath: Buffer to receive the fully-qualified filename for the *
 *                    appropriate language (must already be allocated)       *
 *   PSZ pszName    : Name of the file to look for                           *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if pszName was generated successfully; FALSE otherwise.            *
 * ------------------------------------------------------------------------- */
BOOL GetLanguageFile( PSZ pszFullPath, PSZ pszName )
{
    CHAR  szShortLang[ 3 ],
          chBootVol;
    PSZ   pszLang,
          pszUlsPath;
    ULONG ulBootVol,
          ulRC;


    // Locate the LANGUAGE directory
    ulRC = DosScanEnv("ULSPATH", &pszUlsPath );
    if ( ulRC || !(*pszUlsPath) ) {
        ulRC = DosQuerySysInfo( QSV_BOOT_DRIVE, QSV_BOOT_DRIVE,
                                &ulBootVol, sizeof(ULONG)      );
        if ( ulRC ) return FALSE;
        chBootVol = 64 + ulBootVol;
        sprintf( pszFullPath, "%c:\\LANGUAGE\\", chBootVol );
    } else
        sprintf( pszFullPath, "%s\\", pszUlsPath );

    // Generate the two-letter language code from the current LANG
    memset( szShortLang, 0, sizeof(szShortLang) );
    ulRC = DosScanEnv("LANG", &pszLang );
    if (( ulRC ) || ( ! strlen(pszLang) )) return FALSE;
    if ( strnicmp( pszLang, "ZH_TW", 5 ) == 0 )
        sprintf( szShortLang, "TW");
    else if ( strnicmp( pszLang, "ZH_CN", 5 ) == 0 )
        sprintf( szShortLang, "CX");
    else
        strncpy( szShortLang, pszLang, 2 );

    strncat( pszFullPath, strupr(szShortLang), CCHMAXPATH );
    strncat( pszFullPath, "\\",                CCHMAXPATH );
    strncat( pszFullPath, pszName,             CCHMAXPATH );

    return ( TRUE );
}


/* ------------------------------------------------------------------------- *
 * FontPreviewProc()                                                         *
 *                                                                           *
 * Subclassed window procedure for the font preview static text control.     *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY FontPreviewProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    APIRET rc;
    CHAR   szFont[ FACESIZE + 4 ];

    switch( msg ) {

        case WM_PRESPARAMCHANGED:
            switch ( (ULONG) mp1 ) {
                case PP_FONTNAMESIZE:
                    rc = WinQueryPresParam( hwnd, PP_FONTNAMESIZE, 0, NULL,
                                            sizeof( szFont ), szFont,
                                            QPF_NOINHERIT );
                    if ( rc ) {
                        WinSetWindowText( hwnd, szFont );
                    }
                    break;

                default: break;
            }
            break;

        default: break;
    }

    return (MRESULT) g_pfnTextProc( hwnd, msg, mp1, mp2 );

}


/* ------------------------------------------------------------------------- *
 * HorizontalRuleProc()                                                      *
 *                                                                           *
 * Subclassed window procedure for the "horizontal rule" rectangle control.  *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY HorizontalRuleProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    RECTL  rcl;
    POINTL ptl;
    HPS    hps;

    switch( msg ) {
        case WM_PAINT:
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            WinQueryWindowRect( hwnd, &rcl );

            GpiSetColor( hps, SYSCLR_BUTTONDARK );
            ptl.x = rcl.xLeft;
            ptl.y = rcl.yTop;
            GpiMove( hps, &ptl );
            ptl.x = rcl.xRight-1;
            ptl.y = rcl.yTop;
            GpiLine( hps, &ptl );
            GpiSetColor( hps, SYSCLR_BUTTONLIGHT );
            ptl.x = rcl.xLeft;
            ptl.y = rcl.yTop-1;
            GpiMove( hps, &ptl );
            ptl.x = rcl.xRight-1;
            ptl.y = rcl.yTop-1;
            GpiLine( hps, &ptl );

            WinEndPaint( hps );
            return (MRESULT) 0;

        default: break;
    }

    return (MRESULT) g_pfnRecProc( hwnd, msg, mp1, mp2 );

}


/* ------------------------------------------------------------------------- *
 * InsetBorderProc()                                                         *
 *                                                                           *
 * Subclassed window procedure for the "inset" style rectangle control.      *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY InsetBorderProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    RECTL  rcl;
    HPS    hps;

    switch( msg ) {
        case WM_PAINT:
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            WinQueryWindowRect( hwnd, &rcl );
            DrawInsetBorder( hps, rcl );
            WinEndPaint( hps );
            return (MRESULT) 0;

        default: break;
    }

    return (MRESULT) g_pfnRecProc( hwnd, msg, mp1, mp2 );

}


/* ------------------------------------------------------------------------- *
 * OutlineBorderProc()                                                       *
 *                                                                           *
 * Subclassed window procedure for the "outline" style rectangle control.    *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY OutlineBorderProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    RECTL  rcl;
    HPS    hps;

    switch( msg ) {
        case WM_PAINT:
            hps = WinBeginPaint( hwnd, NULLHANDLE, NULLHANDLE );
            WinQueryWindowRect( hwnd, &rcl );
            GpiCreateLogColorTable( hps, 0, LCOLF_RGB, 0, 0, NULL );
            DrawOutlineBorder( hps, rcl );
            WinEndPaint( hps );
            return (MRESULT) 0;

        default: break;
    }

    return (MRESULT) g_pfnRecProc( hwnd, msg, mp1, mp2 );

}


/* ------------------------------------------------------------------------- *
 * DrawInsetBorder()                                                         *
 *                                                                           *
 * Draws an 'inset' border around the specified rectangle.  Mostly called    *
 * from the WM_PAINT procedure of subclassed controls.                       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HPS hps  : Current presentation space handle                            *
 *   RECTL rcl: Area around which to draw the border                         *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if pszName was generated successfully; FALSE otherwise.            *
 * ------------------------------------------------------------------------- */
void DrawInsetBorder( HPS hps, RECTL rcl )
{
    POINTL ptl;

    GpiSetColor( hps, SYSCLR_BUTTONDARK );
    ptl.x = rcl.xLeft;
    ptl.y = rcl.yBottom;
    GpiMove( hps, &ptl );
    ptl.x = rcl.xLeft;
    ptl.y = rcl.yTop;
    GpiLine( hps, &ptl );
    ptl.x = rcl.xLeft;
    ptl.y = rcl.yTop;
    GpiMove( hps, &ptl );
    ptl.x = rcl.xRight-1;
    ptl.y = rcl.yTop;
    GpiLine( hps, &ptl );
    GpiSetColor( hps, SYSCLR_BUTTONLIGHT );
    ptl.x = rcl.xLeft + 1;
    ptl.y = rcl.yBottom;
    GpiMove( hps, &ptl );
    ptl.x = rcl.xRight-1;
    ptl.y = rcl.yBottom;
    GpiLine( hps, &ptl );
    ptl.x = rcl.xRight-1;
    ptl.y = rcl.yTop-1;
    GpiMove( hps, &ptl );
    ptl.x = rcl.xRight-1;
    ptl.y = rcl.yBottom;
    GpiLine( hps, &ptl );
}


/* ------------------------------------------------------------------------- *
 * DrawNice3DBorder                                                          *
 *                                                                           *
 * Draw an attractive MLE-style 3D border around a control.                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HPS hps  : Handle of the control's presentation space.                  *
 *   RECTL rcl: Rectangle defining the boundaries of the control.            *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void DrawNice3DBorder( HPS hps, RECTL rcl )
{
    POINTL ptl;

    GpiSetColor( hps, SYSCLR_BUTTONLIGHT );
    ptl.x = rcl.xLeft;
    ptl.y = rcl.yBottom;
    GpiMove( hps, &ptl );
    ptl.x = rcl.xRight - 1;
    GpiLine( hps, &ptl );
    ptl.y = rcl.yTop - 1;
    GpiLine( hps, &ptl );
    GpiSetColor( hps, SYSCLR_DIALOGBACKGROUND );
    ptl.x = rcl.xLeft;
    ptl.y = rcl.yBottom + 1;
    GpiMove( hps, &ptl );
    ptl.x = rcl.xRight - 2;
    GpiLine( hps, &ptl );
    ptl.y = rcl.yTop;
    GpiLine( hps, &ptl );
    GpiSetColor( hps, SYSCLR_BUTTONDARK );
    ptl.x = rcl.xLeft;
    ptl.y = rcl.yBottom + 1;
    GpiMove( hps, &ptl );
    ptl.y = rcl.yTop - 1;
    GpiLine( hps, &ptl );
    ptl.x = rcl.xRight - 1;
    GpiLine( hps, &ptl );
    GpiSetColor( hps, CLR_BLACK );
    ptl.x = rcl.xLeft + 1;
    ptl.y = rcl.yBottom + 2;
    GpiMove( hps, &ptl );
    ptl.y = rcl.yTop - 2;
    GpiLine( hps, &ptl );
    ptl.x = rcl.xRight - 3;
    GpiLine( hps, &ptl );
}


/* ------------------------------------------------------------------------- *
 * DrawOutlineBorder()                                                       *
 *                                                                           *
 * Draws a grey outline box around the specified rectangle.                  *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HPS hps  : Current presentation space handle                            *
 *   RECTL rcl: Boundaries defining the box to draw                          *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 *   TRUE if pszName was generated successfully; FALSE otherwise.            *
 * ------------------------------------------------------------------------- */
void DrawOutlineBorder( HPS hps, RECTL rcl )
{
    POINTL ptl;

    GpiSetLineType( hps, LINETYPE_DEFAULT );
    GpiSetColor( hps, 0x808080 );
    ptl.x = rcl.xLeft;
    ptl.y = rcl.yBottom;
    GpiMove( hps, &ptl );
    ptl.x = rcl.xRight - 1;
    ptl.y = rcl.yTop;
    GpiBox( hps, DRO_OUTLINE, &ptl, 0, 0 );
}


/* ------------------------------------------------------------------------- *
 * SetContainerFieldTitle()                                                  *
 *                                                                           *
 * Change the title text of a container field (column) in details view.  The *
 * field must already exist in the container.                                *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND  hwndCnr : Handle of the container                                 *
 *   ULONG ulOffset: Record-structure offset of the field to update          *
 *   PSZ   pszNew  : New field title (a copy will be allocated)              *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void SetContainerFieldTitle( HWND hwndCnr, ULONG ulOffset, PSZ pszNew )
{
    PFIELDINFO fi;          // pointer to current field info

    fi = WinSendMsg( hwndCnr, CM_QUERYDETAILFIELDINFO,
                     0, MPFROMSHORT( CMA_FIRST ));
    while ( fi && fi->offStruct != ulOffset ) {
        fi = fi->pNextFieldInfo;
    }
    if ( fi->pTitleData ) free ( fi->pTitleData );
    fi->pTitleData = strdup( pszNew );
    WinSendMsg( hwndCnr, CM_INVALIDATEDETAILFIELDINFO, 0, 0 );
}


/* ------------------------------------------------------------------------- *
 * MenuItemAddCnd()                                                          *
 *                                                                           *
 * Adds a menu item, conditional on it not existing already.                 *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND  hwndMenu: Handle of the menu                                      *
 *   SHORT sPos    : Position of new item                                    *
 *   SHORT sID     : ID of new item                                          *
 *   PSZ   pszTitle: Title of new item                                       *
 *   SHORT sfStyle : Style flags                                             *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 * ------------------------------------------------------------------------- */
BOOL MenuItemAddCnd( HWND hwndMenu, SHORT sPos, SHORT sID, PSZ pszTitle, SHORT sfStyle )
{
    MENUITEM mi;
    SHORT    rc;

    if ( ! winhQueryMenuItem( hwndMenu, sID, TRUE, &mi )) {
        rc = winhInsertMenuItem( hwndMenu, sPos, sID,  pszTitle, sfStyle, 0 );
        if (( rc != MIT_MEMERROR ) && ( rc != MIT_ERROR ))
            return TRUE;
    }
    return FALSE;
}


/* ------------------------------------------------------------------------- *
 * GetSelectedPartitionOrFreeSpace()                                         *
 *                                                                           *
 * Queries the given diskview control for the partition which currently has  *
 * selection emphasis.                                                       *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *   HWND   hwndDV   : Handle of the DiskView control.                       *
 *   PPVCTLDATA ppvd : Resource ID of the diskview control.                  *
 *                                                                           *
 * RETURNS: BOOL                                                             *
 * ------------------------------------------------------------------------- */
BOOL GetSelectedPartitionOrFreeSpace( HWND hwndDV, PPVCTLDATA ppvd )
{
    HWND      hwndDisk = NULLHANDLE,
              hwndPart = NULLHANDLE;
    WNDPARAMS wndp = {0};
    BOOL      bOK = FALSE;

    if ( !ppvd || !hwndDV ) return FALSE;

    // Find the currently-selected disk+partition
    hwndDisk = (HWND) WinSendMsg( hwndDV, LLM_QUERYDISKEMPHASIS,
                                  MPVOID, MPFROMSHORT( LDV_FS_SELECTED ));
    if ( hwndDisk != NULLHANDLE )
        hwndPart = (HWND) WinSendMsg( hwndDisk, LDM_QUERYPARTITIONEMPHASIS,
                                      MPVOID, MPFROMSHORT( LPV_FS_SELECTED ));

    // Now get the partition information from its control
    if ( hwndPart != NULLHANDLE ) {
        wndp.fsStatus  = WPM_CTLDATA;
        wndp.cbCtlData = sizeof( PVCTLDATA );
        wndp.pCtlData  = ppvd;
        if ( WinSendMsg( hwndPart, WM_QUERYWINDOWPARAMS, MPFROMP( &wndp ), MPVOID ))
            bOK = TRUE;
    }
    return bOK;
}

