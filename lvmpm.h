#define INCL_DOS
#define INCL_DOSRESOURCES
#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#define INCL_DOSFILEMGR
#define INCL_GPI
#define INCL_WIN
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lvmcalls.h"   // LVM declarations (includes lvm_intr.h)
#include "lvm_ctls.h"   // custom controls
#include "utils.h"      // utility functions
#include "ids.h"        // resource IDs

// Stuff from XWPHelpers
#include "include\setup.h"      // basic project setup
#include "helpers\comctl.h"     // split window
#include "helpers\nls.h"        // NLS functions
#include "helpers\gpih.h"       // GPI functions
#include "helpers\winh.h"       // PM functions


/*****************************************************************************
 ** IMPORTED API FUNCTIONS                                                  **
 *****************************************************************************/

// WinHSWITCHfromHAPP() is missing from the toolkit headers
#pragma import ( WinHSWITCHfromHAPP,, "PMMERGE", 5199 )
extern HSWITCH APIENTRY WinHSWITCHfromHAPP(HAPP happ);


/*****************************************************************************
 ** MACROS                                                                  **
 *****************************************************************************/

#define DebugBox( errmsg ) \
    WinMessageBox( HWND_DESKTOP, HWND_DESKTOP, (PSZ)errmsg, "Debug", 0, MB_MOVEABLE | MB_OK | MB_ERROR )


/*****************************************************************************
 ** CONSTANTS                                                               **
 *****************************************************************************/

/* Main window class name
 */
#define SZ_CLASSNAME        "LVMClientWindow"


/* Volumes display (top panel) class name
 */
#define WC_LOGICALVIEW      "LVMVolumePanel"


/* Custom messages
 */
#define LPM_STYLECHANGED    ( WM_USER + 200 )


/* INI keys
 */
#define SZ_INI_APP          "Logical Volume Manager PM"
#define SZ_INI_KEY_POSITION "Position"
#define SZ_INI_KEY_FLAGS    "Preferences"
#define SZ_INI_KEY_FONT_CNR "ContainerFont"
#define SZ_INI_KEY_FONT_VOL "DetailsFont"
#define SZ_INI_KEY_FONT_DSK "DiskFont"
#define SZ_INI_KEY_FONT_STB "StatusFont"
#define SZ_INI_KEY_FONT_DLG "DialogFont"
#define SZ_INI_KEY_SEL_SIZE "SelectorSize"


/* Version constants
 */
#define SZ_VERSION          "1.00"          // version string
#define SZ_BUILD            "007"           // revision/build number
#define SZ_COPYRIGHT        "2011"          // copyright year(s)


/* Names related to file management
 */
#define RESOURCE_LIB        "lvmpmmri.dll"  // PM resource DLL
#define HELP_FILE           "lvmpm.hlp"     // program help file
#define LOG_FILE            "lvmpm.log"     // application log file name
#define ENGINE_LOG_FILE     "lvm.log"       // LVM engine log file name
#define LOG_PATH            "LOGFILES"      // log path environment variable


/* MEMDISK identifiers
 */
#define SERIAL_MEMDISK      1146316146      // Serial number reported by MemDisk
#define STRING_MEMDISK      "[ MemDisk ]"   // Disk name reported by MemDisk
#define STRING_MEMDISK_VOL  "eCS Memory Bootdisk"   // Volume name reported by MemDisk


/* Hardcoded error text (used when resources are unavailable)
 */
#define SZ_ERROR_INIT       "Initialization Error"
#define SZ_ERROR_RESOURCE   "Failed to load resource"
#define SZ_ERROR_GENERIC    "Error"


/* String length constants
 */
#define STRING_RES_MAXZ      256                 // limit for string resources
#define STRING_ERROR_MAXZ    ( STRING_RES_MAXZ ) // limit for error strings
// used for the error message shown when WinStartApp() fails:
#define STRING_APP_ERROR_MAXZ (( STRING_RES_MAXZ * 2 ) + 9 )


/* Program-specific flag values used in various data structures
 */

// Used to mark changes made by secondary dialogs (not used in the main window):
#define FS_CHANGED          0x01        // changes were made in this context

// Global flags
#define FS_APP_DBCS         0x02        // we are running on a DBCS system
#define FS_APP_LOGGING      0x04        // logging is enabled

// These correspond to user-configured preferences:
#define FS_APP_IBMTERMS     0x100       // use IBM terminology for volume types
#define FS_APP_IECSIZES     0x200       // use IEC terminology for binary *bytes
#define FS_APP_BOOTWARNING  0x400       // warn on exit if no bootable OS/2 volumes
#define FS_APP_PMSTYLE      0x800       // use a more PM-like visual style
// Boot Manager configuration will always be offered if it is already installed
#define FS_APP_ENABLE_BM    0x1000      // enable Boot Manager install
// Air-Boot will never be enabled if the install program cannot be found
#define FS_APP_ENABLE_AB    0x2000      // enable Air-Boot
#define FS_APP_HIDE_FREEPRM 0x4000      // don't show empty PRM drives
#define FS_APP_HIDE_NONLVM  0x8000      // don't show non-LVM-managed volumes
#define FS_APP_PREFERENCES  0xFF00      // mask of all preference bits

/* LVM-engine-specific flag values (used in the fsEngine field of DVMGLOBAL)
 */
#define FS_ENGINE_PENDING   0x01        // Uncommitted LVM changes are pending
#define FS_ENGINE_REFRESH   0x02        // A refresh of LVM data is required
#define FS_ENGINE_BOOTMGR   0x10        // IBM Boot Manager is installed
#define FS_ENGINE_AIRBOOT   0x20        // AiR-BOOT is installed


/* These values are used in the bType field of DVMCREATEPARMS
 */

// Requested volume type when creating a volume
#define VOLUME_TYPE_STANDARD    1
#define VOLUME_TYPE_ADVANCED    2

// Requested partition type when creating a partition
#define PARTITION_TYPE_PRIMARY  1
#define PARTITION_TYPE_LOGICAL  2


/* Amount of safety margin (in MiB) to subtract from a partition that extends
 * to 512 GiB
 */
#define US_MARGIN_512               21


/* Default Boot Manager timeout
 */
#define US_BMGR_DEF_TIMEOUT         30

/* Values returned by BootMgrPopulateMenu
 */
#define BOOTMGR_MENU_EMPTY          -1      // boot menu is empty
#define BOOTMGR_MENU_NODEF          -2      // boot menu has no default set


/* Minimum size (in pixels) for the top and bottom split panels
#define US_SPLIT_MIN_PX             200
 */


/* Number of detail view fields in various containers
 */
#define VOLUME_CNR_FIELDS           6
#define PARTITION_CNR_FIELDS        3
#define EXPAND_CNR_FIELDS           4

/* Program initialization values (for error checking)
 */
#define INITFAILURE         -1
#define APPNORMAL           0
#define USERQUIT            1


/* Program return code values (for use on exit)
 */
#define RETURN_NORMAL       0x000000      // Completed; no action required
#define RETURN_REBOOT       0x010000      // Completed; changes require a reboot
#define RETURN_ABORT        0x100000      // Cancelled; abort installation
#define RETURN_INITFAIL     0x111111      // Error; program initalization failed


/*****************************************************************************
 ** DATA TYPES                                                              **
 *****************************************************************************
 * This program's internally-defined structures use the prefix "(P)DVM"      *
 * (short for "Disk and Volume Manager").  The prefix "(P)LVM" is used by    *
 * structures defined in lvmcalls.h.  (The custom controls in lvm_ctls.c use *
 * their own naming convention for their structures.)                        *
 *****************************************************************************/

/* This structure is used for (detail-view) records in the volumes container.
 */
typedef struct _Volume_Record {
    MINIRECORDCORE record;      // standard mini-record data
    ADDRESS        handle;      // volume's LVM handle
    ULONG          ulVolume,    // index (in global array) of current volume
                   ulSize;      // size field value
    PSZ            pszLetter,   // letter field text
                   pszName,     // name field text
                   pszFS,       // filesystem field text
                   pszType,     // volume-type field text
                   pszFlags;    // flags field text
} DVMVOLUMERECORD, *PDVMVOLUMERECORD;


/* This structure is used for (detail-view) records in the partitions container
 * (located on the current-volume information/status panel).
 */
typedef struct _Partition_Record {
    MINIRECORDCORE record;      // standard mini-record data
    ADDRESS        handle,      // partition's LVM handle
                   hDisk;       // LVM handle of partition's disk
    ULONG          ulSize,      // size field value
                   ulDisk;      // LVM disk number
    PSZ            pszName;     // name field text
} DVMPARTITIONRECORD, *PDVMPARTITIONRECORD;


/* This structure contains window data for the volume display (a.k.a. "logical
 * view") panel.
 */
typedef struct _Volume_Panel_Ctl_Data {
#if 0
    HAB     hab;                        // anchor-block handle
    HMODULE hmri;                       // resource library handle
#endif
    HWND    hwndVolumes,                // volumes container
            hwndInfo;                   // current-volume info panel
} DVMLOGVIEWCTLDATA, *PDVMLOGVIEWCTLDATA;


/* This structure is used to pass data to and from the volume name dialog.
 */
typedef struct _Volume_Name_Params {
    HAB     hab;                        // anchor block handle
    HMODULE hmri;                       // resource library handle
    ADDRESS handle;                     // LVM handle of the volume
    BYTE    bType;                      // LVM object type (partition or volume)
    PSZ     pszName;                    // volume name
    PSZ     pszFS;                      // volume filesystem
    CHAR    cLetter;                    // volume drive letter
    CHAR    szFontDlgs[ FACESIZE+4 ];   // dialog font
    USHORT  fsProgram;                  // program-related flags
} DVMVOLUMEPARAMS, *PDVMVOLUMEPARAMS;


/* This structure is used to pass options to and from the disk properties
 * dialog.
 */
typedef struct _Disk_Name_Params {
    HAB     hab;                        // anchor block handle
    HMODULE hmri;                       // resource library handle
    ADDRESS handle;                     // LVM handle of the disk drive
    PSZ     pszName;                    // user-definable name of the disk drive
    CHAR    achSerial[ 12 ];            // factory serial number of the disk drive
    CHAR    szFontDlgs[ FACESIZE+4 ];   // dialog font
    USHORT  fsProgram;                  // program-related flags
    BOOLEAN fAccessible;                // indicates whether the disk is accessible
} DVMDISKPARAMS, *PDLVMDISKPARAMS;


/* This structure is used to pass options to and from the volume- and partition-
 * creation dialogs.
 */
typedef struct _Creation_Params {
    HAB        hab;                     // anchor block handle
    HMODULE    hmri;                    // resource library handle
    USHORT     fsProgram;               // program-related flags
    USHORT     fsEngine;                // LVM-related flags
    BOOL       fBootable;               // bootable/startable requested
    BYTE       bType;                   // requested partition or volume type
    PSZ        pszName;                 // name of new volume/partition
    PADDRESS   pPartitions;             // array of partition(s) to use
    ULONG      ulPartitions;            // number of partitions specified
    CHAR       cLetter;                 // requested letter of new volume
    CHAR       szFontDlgs[ FACESIZE+4 ],   // dialog font
               szFontDisks[ FACESIZE+4 ];  // disk list font

    COUNTRYSETTINGS ctry;               // NLS country settings (from XWPHelpers)
    PLVMDISKINFO    disks;              // array of all disk drives
    CARDINAL32      ulDisks;            // number of disks

} DVMCREATEPARMS, *PDVMCREATEPARMS;


/* This structure is used to pass data to and from the Boot Manager options
 * dialog.
 */
typedef struct _BootMgr_Params {
    HAB     hab;                        // anchor-block handle
    HMODULE hmri;                       // resource library handle
    ULONG   fsProgram;                  // various program-related flags
    CHAR    szFontDlgs[ FACESIZE+4 ];   // dialog font
} DVMBOOTMGRPARAMS, *PDVMBOOTMGRPARAMS;


/* This data structure contains global data used throughout the program.  It is
 * accessed via a pointer stored in the main client window's window words.
 */
typedef struct _Program_Global_Data {

    // Global objects needed throughout the application
    HAB             hab;                // anchor-block handle
    HMODULE         hmri;               // resource library handle
    COUNTRYSETTINGS ctry;               // NLS country settings (from XWPHelpers)

    // UI management and configuration data
    FILE     *pLog;                     // debug log file
    HWND     hwndMenu,                  // handle of main menubar
             hwndSplit,                 // handle of split-window panel
             hwndVolumes,               // handle of volumes (top) panel
             hwndDisks,                 // handle of disks (bottom) panel
             hwndTT,                    // tooltip control for the disk list
             hwndPopupDisk,             // popup context menu for disks
             hwndPopupPartition,        // popup context menu for partitions
             hwndPopupVolume;           // popup context menu for volumes
    LONG     lStatusHeight,             // height of the status bar(s)
             lStatusDiv,                // width of the left status bar
             lStatusWidth;              // total width of both status bars
    HAPP     happAB;                    // HAPP of AiR-BOOT installer
    USHORT   fsProgram;                 // various app-specific flags
    CHAR     szABEXE[ CCHMAXPATH+1 ],   // name of AiR-BOOT install program
             szFontMain[ FACESIZE+4 ],  // main window (a.k.a. statusbar) font
             szFontCnr[ FACESIZE+4 ],   // volume container font
             szFontInfo[ FACESIZE+4 ],  // volume info panel font
             szFontDisks[ FACESIZE+4 ], // disk list font
             szFontDlgs[ FACESIZE+4 ];  // dialog font
    HPOINTER hptrStandard,              // icon for normal compatibility volumes
             hptrAdvanced,              // icon for LVM volumes
             hptrCD,                    // icon for CD/DVD/BD volumes
             hptrLAN,                   // icon for non-LVM network volumes
             hptrUnknown;               // icon for unknown non-LVM volumes
    ULONG    ulReturn;                  // program return code

    // LVM-specific data
    PLVMDISKINFO   disks;               // array of all disk drives
    PLVMVOLUMEINFO volumes;             // array of all logical volumes
    CARDINAL32     ulDisks,             // number of disks
                   ulVolumes;           // number of volumes
    USHORT         fsEngine;            // various LVM-specific flags

} DVMGLOBAL, *PDVMGLOBAL;



/*****************************************************************************
 ** SUBCLASSED WINDOW PROCEDURES                                           **
 *****************************************************************************/

PFNWP g_pfnRecProc;             // Default SS_FGNDFRAME window procedure
PFNWP g_pfnTextProc;            // Default SS_TEXT window procedure


/*****************************************************************************
 ** FUNCTION PROTOTYPES                                                     **
 *****************************************************************************/

// Functions in lvmpm.c
void             ChangeSizeDisplay( HWND hwnd, PDVMGLOBAL pGlobal );
void             ChangeVolumeTypeDisplay( HWND hwnd, PDVMGLOBAL pGlobal );
void             DiskListClear( PDVMGLOBAL pGlobal );
void             DiskListPopulate( HWND hwnd );
MRESULT EXPENTRY FontsDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
void             Log_DiskInfo( PDVMGLOBAL pGlobal );
void             Log_VolumeInfo( PDVMGLOBAL pGlobal );
FILE           * LogFileInit( void );
void             LVM_FreeData( PDVMGLOBAL pGlobal );
BOOL             LVM_InitData( HWND hwnd, PDVMGLOBAL pGlobal );
void             LVM_Refresh( HWND hwnd );
BOOL             LVM_Start( FILE *pLog, HAB hab, HMODULE hmri );
void             LVM_Stop( PDVMGLOBAL pGlobal );
void             MainWindowCleanup( HWND hwnd );
void             MainWindowFocus( HWND hwnd, BOOL fNext );
void             MainWindowInit( HWND hwnd );
void             MainWindowSetFonts( HWND hwnd, PDVMGLOBAL pGlobal );
void             MainWindowSize( HWND hwnd, LONG lWidth, LONG lHeight, PDVMGLOBAL pGlobal );
MRESULT EXPENTRY MainWndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
void             PartitionContainerClear( HWND hwndCnr, PDVMGLOBAL pGlobal );
void             PartitionContainerDestroy( PDVMGLOBAL pGlobal );
void             PartitionContainerPopulate( HWND hwndCnr, Partition_Information_Array pia, PDVMGLOBAL pGlobal );
void             PartitionContainerSetup( PDVMGLOBAL pGlobal );
void             PopupEngineError( PSZ pszMessage, CARDINAL32 code, HWND hwnd, HAB hab, HMODULE hmri );
MRESULT EXPENTRY PrefsDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY ProdInfoDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
void             SelectFont( HWND hwnd, USHORT usID );
void             SetAvailableActions( HWND hwnd );
void             SetBootMgrActions( PDVMGLOBAL pGlobal );
void             SetModified( HWND hwnd, BOOL fModified );
void             Settings_Load( PDVMGLOBAL pGlobal, PLONG pX, PLONG pY, PLONG pW, PLONG pH );
void             Settings_Save( HWND hwndFrame, PDVMGLOBAL pGlobal );
void             Status_Clear( HWND hwnd );
void             Status_Partition( HWND hwnd, HWND hPart );
void             Status_Volume( HWND hwnd, PDVMVOLUMERECORD pRec );
void             System_RewriteMBR( HWND hwnd );
void             VolumeContainerClear( PDVMGLOBAL pGlobal );
void             VolumeContainerDestroy( PDVMGLOBAL pGlobal );
void             VolumeContainerPopulate( PDVMGLOBAL pGlobal );
void             VolumeContainerSetup( PDVMGLOBAL pGlobal );
void             VolumeContainerSelect( HWND hwnd, PDVMVOLUMERECORD pRec );
MRESULT EXPENTRY VolumesPanelProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

// Functions in airboot.c
void             AirBoot_Delete( HWND hwnd );
void             AirBoot_Install( HWND hwnd );
void             AirBoot_InstallerExit( HWND hwnd, ULONG rc, PDVMGLOBAL pGlobal );
BOOL             AirBoot_IsInstalled( PCARDINAL32 pError );
BOOL             AirBoot_GetInstaller( PDVMGLOBAL pGlobal );

// Functions in bootmgr.c
BOOL             BootMgrInstall( HWND hwnd, CARDINAL32 disk );
void             BootMgrOptions( HWND hwnd );
MRESULT EXPENTRY BootMgrOptionsProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
SHORT            BootMgrPopulateMenu( HWND hwnd, ADDRESS hDefault, PDVMBOOTMGRPARAMS pData );
void             BootMgrRemove( HWND hwnd );

// Functions in volumes.c
void             VolumeAddPartition( HWND hwnd );
BOOL             VolumeCreate( HWND hwnd, PDVMGLOBAL pGlobal );
MRESULT EXPENTRY VolumeCreate1WndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY VolumeCreate2WndProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
void             VolumeCreate2Resize( HWND hwnd, SHORT usW, SHORT usH, BOOL fMulti );
PSZ              VolumeDefaultName( PSZ pszName );
BOOL             VolumeNameExists( PSZ pszName, Volume_Control_Array volumes );
BOOL             VolumePartitionIsAdded( HWND hwnd, PVCTLDATA partinfo );
void             VolumePopulateDisks( HWND hwndCtl, PDVMCREATEPARMS pData );
CARDINAL32       VolumePopulateLetters( HWND hwndLB, HAB hab, HMODULE hmri );
void             VolumeRemovePartition( HWND hwnd );



