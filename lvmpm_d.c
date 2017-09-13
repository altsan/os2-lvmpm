#include "lvmpm.h"

/* ------------------------------------------------------------------------- *
 * main()                                                                    *
 * ------------------------------------------------------------------------- */
int main( int argc, char *argv[] )
{
    DVMWINDOWPARAMS init = {0};     // window params for the main dialog window

    QMSG     qmsg;                                 // message queue
    HWND     hwndMain,                             // main program dialog window
             hwndHelp;                             // help instance handle
    HACCEL   hAccel;                               // accel-table handle
    HELPINIT help;                                 // help init structure
    CHAR     szMRI[ CCHMAXPATH+1 ]        = {0},   // resource DLL name
             szRes[ STRING_RES_MAXZ ]     = {0},   // string resource buffer
             szError[ STRING_ERROR_MAXZ ] = {0};   // error buffer
    BOOL     bInitErr = FALSE,                     // did initialization fail?
             bQuit    = FALSE;                     // program exit was confirmed
    ULONG    i;


    init.ulReturn = RETURN_INITFAIL;

    /* Parse the command-line arguments, if any.
     */
    for ( i = 1; i < argc; i++ ) {
        if ( strnicmp( argv[ i ], "/log", 4 ) == 0 ) {
            init.fsProgram |= FS_APP_LOGGING;
        }
    }

    /* Do the necessary Presentation Manager initialization for the application.
     * Note: Any error messages required prior to successful loading of the
     * resource library must be hard-coded (English).  Once the resources have
     * been loaded successfully, all text should be loaded as string resources.
     */
    init.hab = WinInitialize( 0 );
    if ( !init.hab ) {
        sprintf( szError, "WinInitialize() failed.");
        bInitErr = TRUE;
    }
    else if (( init.hmq = WinCreateMsgQueue( init.hab, 0 )) == NULLHANDLE ) {
        sprintf( szError, "Unable to create message queue: PM error code 0x%X\n",
                 ERRORIDERROR( WinGetLastError( init.hab )));
        bInitErr = TRUE;
    }

    /* Now try to initialize the application data.
     */
    if ( !bInitErr ) {
        // Find out if we're running on a DBCS system
        if ( CheckDBCS() ) init.fsProgram |= FS_APP_DBCS;

        // Try to load the resource library.  Look in %ULSPATH%\<lang> first...
        if ( ! GetLanguageFile( szMRI, RESOURCE_LIB ) ||
            (( DosLoadModule( (PSZ) szError, sizeof( szError ),
                              szMRI, &(init.hmri) )) != NO_ERROR ))
        {
            // Failing that, try loading it from the LIBPATH like any other DLL
            if (( DosLoadModule( (PSZ) szError, sizeof( szError ),
                  RESOURCE_LIB, &(init.hmri) )) != NO_ERROR )
            {
                sprintf( szError,
                         "Unable to locate resource library \"%s\":\r\nThe PM error code is 0x%X\n",
                         RESOURCE_LIB, ERRORIDERROR( WinGetLastError( init.hab )));
                bInitErr = TRUE;
            }
        }

        // Register our custom disk list control
        if ( !bInitErr ) {
            if ( ! DLRegisterClass( init.hab )) {
                if ( WinLoadString( init.hab, init.hmri, IDS_ERROR_REGISTER,
                                    STRING_RES_MAXZ, szRes ))
                    sprintf( szError, szRes, WC_LVMDISKS,
                             ERRORIDERROR( WinGetLastError( init.hab )));
                else
                    sprintf( szError,
                             "Failed to register class %.40s:\r\nThe PM error code is 0x%08X\n",
                             WC_LVMDISKS, ERRORIDERROR( WinGetLastError( init.hab )));
                bInitErr = TRUE;
            }
        }
    }

    /* Load the main program dialog.
     */
    if ( !bInitErr && ( hwndMain = WinLoadDlg( HWND_DESKTOP, HWND_DESKTOP,
                                               (PFNWP) MainDialogProc,
                                               init.hmri, ID_MAINPROGRAM, &init )
                      ) == NULLHANDLE )
    {
        if ( WinLoadString( init.hab, init.hmri, IDS_ERROR_INITDLG,
                            STRING_RES_MAXZ, szRes ))
            sprintf( szError, szRes,
                     ERRORIDERROR( WinGetLastError( init.hab )));
        else
            sprintf( szError,
                     "Failed to load the main window dialog:\r\nThe PM error code is 0x%08X\n",
                     ERRORIDERROR( WinGetLastError( init.hab )));
        bInitErr = TRUE;
    }

    /* If any initialization error occurred up to this point, display the
     * predetermined error message and then exit.
     */
    if ( bInitErr )
    {
        WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, szError,
                       SZ_ERROR_INIT, 0, MB_MOVEABLE | MB_OK | MB_ERROR );
    }
    else {
        // We're in business!
        init.ulReturn = RETURN_NORMAL;

        // initialize keyboard shortcuts
        hAccel = WinLoadAccelTable( init.hab, init.hmri, ID_MAINPROGRAM );
        WinSetAccelTable( init.hab, hAccel, hwndMain );

        // initialize the online help
        WinLoadString( init.hab, init.hmri, IDS_HELP_TITLE, STRING_RES_MAXZ, szRes );
        help.cb                       = sizeof( HELPINIT );
        help.pszTutorialName          = NULL;
        help.phtHelpTable             = (PHELPTABLE) MAKELONG( ID_MAINPROGRAM, 0xFFFF );
        help.hmodHelpTableModule      = init.hmri;
        help.hmodAccelActionBarModule = NULLHANDLE;
        help.fShowPanelId             = CMIC_SHOW_PANEL_ID;
        help.idAccelTable             = 0;
        help.idActionBar              = 0;
        help.pszHelpWindowTitle       = szRes;
        help.pszHelpLibraryName       = HELP_FILE;
/*
        if ( GetLanguageFile( szHLP, HELP_FILE )) {
            rc = DosFindFirst( szHLP, &hdir, FILE_NORMAL, &fb, sizeof(fb), &ulFC, FIL_STANDARD );
            if ( rc == NO_ERROR ) helpInit.pszHelpLibraryName = szHLP;
        }
*/
        hwndHelp = WinCreateHelpInstance( init.hab, &help );
        if ( ! WinAssociateHelpInstance( hwndHelp, hwndMain )) {
            WinLoadString( init.hab, init.hmri, IDS_HELP_LOAD_ERROR, STRING_RES_MAXZ, szError );
            WinLoadString( init.hab, init.hmri, IDS_HELP_ERROR_TITLE, STRING_RES_MAXZ, szRes );
            WinMessageBox( HWND_DESKTOP, hwndMain, szError, szRes,
                           0, MB_MOVEABLE | MB_OK | MB_ERROR );
        }

        // main program loop
        while ( !bQuit ) {
            // loop until quit signalled
            while ( WinGetMsg( init.hab, &qmsg, 0, 0, 0 ))
                WinDispatchMsg( init.hab, &qmsg );

            /* Send a message asking the user to confirm exiting if there were
             * changes made.  (Granted, it's a bit obnoxious to do this on
             * WM_QUIT rather than WM_CLOSE, but partitioning is such a critical
             * operation that it's probably justified in this case.)
             */
            if ( (ULONG) WinSendMsg( hwndMain, WM_COMMAND,
                                     MPFROM2SHORT( ID_VERIFYQUIT, 0 ), 0 )
                         != MBID_CANCEL )
                bQuit = TRUE;

            // cancel quit if necessary
            if ( qmsg.hwnd == NULLHANDLE && !bQuit )
                WinCancelShutdown( init.hmq, FALSE );
        }
        if ( hwndHelp )
            WinDestroyHelpInstance( hwndHelp );
    }

    WinDestroyWindow( hwndMain );

    // reboot the system if necessary
    if ( init.ulReturn == RETURN_REBOOT )
        WinShutdownSystem( init.hab, init.hmq );

    WinDestroyMsgQueue( init.hmq );
    WinTerminate( init.hab );

    return ( init.ulReturn );
}


/* ------------------------------------------------------------------------- *
 * MainDialogProc()                                                          *
 *                                                                           *
 * This is the event handler for the main application dialog window.         *
 * See OS/2 PM reference for a description of input and output.              *
 * ------------------------------------------------------------------------- */
MRESULT EXPENTRY MainDialogProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
    static PDVMWINDOWPARAMS pInit;
    PDVMGLOBAL              pGlobal;
//  PRECORDCORE             pRec;
//  PNOTIFYRECORDEMPHASIS   pNotify;

    HWND        hwndHelp;                   // program help instance
    CARDINAL32  ulError;                    // LVM error code
    ULONG       ulChoice,                   // user message-box selection
                rc;                         // return code
    BOOLEAN     fSaved;                     // LVM changes committed OK
    BOOL        fShow;                      // show/hide control
    PSWP        pswp;                       // current window position
    CHAR        szRes1[ STRING_RES_MAXZ ],
                szRes2[ STRING_RES_MAXZ ],
                szError[ STRING_ERROR_MAXZ ];
    PSZ         pszLogPath,
                pszLogFile;


    switch( msg ) {

        case WM_INITDLG :

            // Initialize our global data structure and store its pointer
            pInit   = (PDVMWINDOWPARAMS) mp2;
            pGlobal = (PDVMGLOBAL) calloc( 1, sizeof( DVMGLOBAL ));
            pGlobal->hab       = pInit->hab;
            pGlobal->hmri      = pInit->hmri;
            pGlobal->fsProgram = pInit->fsProgram;
            WinSetWindowPtr( hwnd, 0, pGlobal );

/*
            if ( parms->logging ) {
                if ( DosScanEnv( LOG_PATH, &pszLogPath ) == NO_ERROR ) {
                    pszLogFile = (PSZ) malloc( strlen(pszLogPath) + strlen(LOG_FILE) + 2 );
                    sprintf( pszLogFile, "%s\\%s", pszLogPath, LOG_FILE );
                    global->logFile = fopen( pszLogFile, "w");
                } else global->logFile = fopen( LOG_FILE, "w");
            } else global->logFile = NULL;
 */

            MainWindowSetup( hwnd, pGlobal );

            return (MRESULT) FALSE;
            // WM_INITDLG end


        case WM_CLOSE:
            WinPostMsg( hwnd, WM_QUIT, 0, 0 );
            return (MRESULT) 0;


        case WM_COMMAND:
            switch( SHORT1FROMMP( mp1 )) {

                case ID_LVM_EXIT:
                    WinPostMsg( hwnd, WM_QUIT, 0, 0 );
                    break;


                case ID_HELP_GENERAL:
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    if (( hwndHelp = WinQueryHelpInstance( hwnd )) != NULLHANDLE )
                        WinSendMsg( hwndHelp, HM_EXT_HELP, 0, 0 );
                    break;


                case ID_VERIFYQUIT:            // Verify quit (called by WM_QUIT)
                    pGlobal = WinQueryWindowPtr( hwnd, 0 );
                    ulChoice = MBID_NO;

/*
                    // See if there is at least one bootable/startable OS/2 volume
                    if ( !CheckBootable( global )) {
                        // No there isn't.  Warn the user
                        WinLoadString( global->hab, global->resources,
                                       STRING_NOBOOTTEXT, STRING_RESOURCE_LIMIT, mbText );
                        WinLoadString( global->hab, global->resources,
                                       STRING_NOBOOTTITLE, STRING_RESOURCE_LIMIT, mbTitle );
                        choice = WinMessageBox( HWND_DESKTOP, hwnd, mbText, mbTitle,
                                                WARNDLG_NO_BOOTABLE,
                                                MB_YESNO | MB_WARNING | MB_MOVEABLE );
                        if ( choice != MBID_YES )
                            return (MRESULT) MBID_CANCEL;
                    }
*/
                    if ( pGlobal->fsEngine & FS_ENGINE_PENDING ) {
                        WinLoadString( pGlobal->hab, pGlobal->hmri,
                                       IDS_SAVE_QUIT, STRING_RES_MAXZ, szRes1 );
                        WinLoadString( pGlobal->hab, pGlobal->hmri,
                                       IDS_SAVE_TITLE, STRING_RES_MAXZ, szRes2 );
                        ulChoice = WinMessageBox( HWND_DESKTOP, hwnd,
                                                  szRes1, szRes2, 0,
                                                  MB_YESNOCANCEL |
                                                   MB_QUERY | MB_MOVEABLE );
                        if ( ulChoice == MBID_YES ) {
                            fSaved = LvmCommitChanges( &ulError );
                            if ( fSaved ) {
                                // Changes saved; now see if a reboot is needed
                                if ( LvmRebootRequired() ) {
                                    WinLoadString( pGlobal->hab, pGlobal->hmri,
                                                   IDS_REBOOT_NOTIFY, STRING_RES_MAXZ, szRes1 );
                                    WinLoadString( pGlobal->hab, pGlobal->hmri,
                                                   IDS_REBOOT_TITLE, STRING_RES_MAXZ, szRes2 );
                                    WinMessageBox( HWND_DESKTOP, hwnd, szRes1, szRes2,
                                                   0, MB_OK | MB_WARNING | MB_MOVEABLE  );
                                    pInit->ulReturn = RETURN_REBOOT;
                                }
                            }
                            else {
                                // Error during save!  Hope this doesn't happen.
                                PopupEngineError( NULL, ulError, hwnd,
                                                  pGlobal->hab, pGlobal->hmri );
                                pInit->ulReturn = ulError;
                            }
                        }
                    }
                    return (MRESULT) ulChoice;
                    // ID_VERIFYQUIT end

                default: break;

            }
            return (MRESULT) 0;
            // WM_COMMAND end


        case WM_CONTROL:
            switch( SHORT2FROMMP( mp1 )) {

                case CN_HELP:
                    // Workaround for WM_HELP getting eaten by the container(s)
                    WinSendMsg( hwnd, WM_HELP,
                                MPFROMSHORT( ID_HELP_GENERAL ),
                                MPFROM2SHORT( CMDSRC_OTHER, FALSE ));
                    return (MRESULT) 0;

                default: break;
            }
            break;
            // WM_CONTROL end


        case WM_DESTROY:
            pGlobal = WinQueryWindowPtr( hwnd, 0 );
            if ( !pGlobal ) break;

//            ClearDriveData( hwnd );
            if ( pGlobal->hwndPopupDisk )
                WinDestroyWindow( pGlobal->hwndPopupDisk );
            if ( pGlobal->hwndPopupPartition )
                WinDestroyWindow( pGlobal->hwndPopupPartition );
            if ( pGlobal->hwndPopupVolume )
                WinDestroyWindow( pGlobal->hwndPopupVolume );
            if ( pGlobal->pLog )
                fclose( pGlobal->pLog );
            free( pGlobal );

            WinSetWindowPtr( hwnd, 0, NULL );
            break;


        case WM_HELP:
            break;


        case WM_MINMAXFRAME:
            /* Show/hide controls in the bottom-left corner of the dialog.
             * (Necessary to avoid overlaying the minimized icon, because
             * WinDefDlgProc doesn't take care of this automatically.)
             */
            pswp = (PSWP) mp1;
            fShow = ( pswp->fl & SWP_MINIMIZE ) ? FALSE : TRUE;
            WinShowWindow( WinWindowFromID( hwnd, IDD_STATUS_LEFT ),     fShow );
            WinShowWindow( WinWindowFromID( hwnd, IDD_STATUS_SELECTED ), fShow );
            WinShowWindow( WinWindowFromID( hwnd, IDD_DISKLIST ),        fShow );
            WinShowWindow( WinWindowFromID( hwnd, IDD_DISK_CANVAS ),     fShow );
            return (MRESULT) FALSE;

/*
        case HM_ERROR:
            DebugBox("Help error.");
             break;
 */

        default: break;

    }

    return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}


/* ------------------------------------------------------------------------- *
 * MainWindowSetup()                                                         *
 *                                                                           *
 * Sets up the main window with stuff like menu & icon, handle any required  *
 * control subclassing, etc.                                                 *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *     HWND hwnd: handle of the main program dialog window                   *
 *                                                                           *
 * RETURNS: N/A                                                              *
 * ------------------------------------------------------------------------- */
void MainWindowSetup( HWND hwnd, PDVMGLOBAL pGlobal )
{
    HPOINTER hIcon;

    hIcon = WinLoadPointer( HWND_DESKTOP, pGlobal->hmri, ID_MAINPROGRAM );

    WinSendMsg( hwnd, WM_SETICON, (MPARAM) hIcon, NULL );
    WinLoadMenu( hwnd, pGlobal->hmri, ID_MAINPROGRAM );
    WinSendMsg( hwnd, WM_UPDATEFRAME, (MPARAM) FCF_MENU, NULL );

    g_pfnRecProc = WinSubclassWindow( WinWindowFromID( hwnd, IDD_VOL_CANVAS ),
                                      CanvasProc );
    WinSubclassWindow( WinWindowFromID( hwnd, IDD_DISK_CANVAS ),
                       CanvasProc );
    WinSubclassWindow( WinWindowFromID( hwnd, IDD_VOL_DIVIDER ),
                       HorizDividerProc );
    WinSubclassWindow( WinWindowFromID( hwnd, IDD_STATUS_LEFT ),
                       InsetBorderProc );
    WinSubclassWindow( WinWindowFromID( hwnd, IDD_STATUS_RIGHT ),
                       InsetBorderProc );

    // Create the context menus
    //global->hwndPopupDisk = WinLoadMenu( HWND_DESKTOP, pGlobal->hmri, IDM_DISKPOPUP );

    //SetupLVMContainers( hwnd );

    CentreWindow( hwnd, NULLHANDLE, SWP_SHOW | SWP_ACTIVATE );

}


/* ------------------------------------------------------------------------- *
 * PopupEngineError()                                                        *
 *                                                                           *
 * Displays message-box for LVM API error messages.                          *
 *                                                                           *
 * ARGUMENTS:                                                                *
 *     PSZ pszMessage : The error message to display, or NULL to have this   *
 *                      function look it up from the error code              *
 *     CARDINAL32 code: The LVM engine error code                            *
 *     HWND hwnd      : Handle of the window producing the message-box       *
 *     HAB hab:       : Anchor-block for the application                     *
 *     HMODULE hmri   : Handle of the resource containing NLV stringtables   *
 *                                                                           *
 * RETURNS: n/a                                                              *
 * ------------------------------------------------------------------------- */
void PopupEngineError( PSZ pszMessage, CARDINAL32 code, HWND hwnd, HAB hab, HMODULE hmri )
{
    CHAR  szRes[ STRING_RES_MAXZ ];
    PSZ   pszTitle;
    ULONG cch,
          ulStyle;

    cch = WinLoadString( hab, hmri, IDS_ERROR_ENGINE, STRING_RES_MAXZ, szRes );
    pszTitle = (PSZ) malloc( cch + 4 );
    if ( pszTitle )
        sprintf( pszTitle, szRes, code );

    if ( !pszMessage ) {
        WinLoadString( hab, hmri, IDS_ERROR_LVM, STRING_RES_MAXZ, szRes );
        pszMessage = szRes;
    }

    ulStyle = MB_MOVEABLE | MB_APPLMODAL | MB_OK | MB_HELP | MB_ERROR;
    WinMessageBox( HWND_DESKTOP, hwnd, pszMessage,
                   ( pszTitle ? pszTitle : ""),
                   IDD_ENGINE_ERROR + code, ulStyle );

    if ( pszTitle ) free( pszTitle );
}



