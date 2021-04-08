#pragma once
// Copyright (c) .NET Foundation and contributors. All rights reserved. Licensed under the Microsoft Reciprocal License. See LICENSE.TXT file in the project root for full license information.


#ifdef __cplusplus
extern "C" {
#endif

#define ReleaseTheme(p) if (p) { ThemeFree(p); p = NULL; }

typedef HRESULT(CALLBACK *PFNTHM_EVALUATE_VARIABLE_CONDITION)(
    __in_z LPCWSTR wzCondition,
    __out BOOL* pf,
    __in_opt LPVOID pvContext
    );
typedef HRESULT(CALLBACK *PFNTHM_FORMAT_VARIABLE_STRING)(
    __in_z LPCWSTR wzFormat,
    __inout LPWSTR* psczOut,
    __in_opt LPVOID pvContext
    );
typedef HRESULT(CALLBACK *PFNTHM_GET_VARIABLE_NUMERIC)(
    __in_z LPCWSTR wzVariable,
    __out LONGLONG* pllValue,
    __in_opt LPVOID pvContext
    );
typedef HRESULT(CALLBACK *PFNTHM_SET_VARIABLE_NUMERIC)(
    __in_z LPCWSTR wzVariable,
    __in LONGLONG llValue,
    __in_opt LPVOID pvContext
    );
typedef HRESULT(CALLBACK *PFNTHM_GET_VARIABLE_STRING)(
    __in_z LPCWSTR wzVariable,
    __inout LPWSTR* psczValue,
    __in_opt LPVOID pvContext
    );
typedef HRESULT(CALLBACK *PFNTHM_SET_VARIABLE_STRING)(
    __in_z LPCWSTR wzVariable,
    __in_z_opt LPCWSTR wzValue,
    __in BOOL fFormatted,
    __in_opt LPVOID pvContext
    );

typedef enum THEME_ACTION_TYPE
{
    THEME_ACTION_TYPE_BROWSE_DIRECTORY,
    THEME_ACTION_TYPE_CHANGE_PAGE,
    THEME_ACTION_TYPE_CLOSE_WINDOW,
} THEME_ACTION_TYPE;

typedef enum THEME_CONTROL_DATA
{
    THEME_CONTROL_DATA_HOVER = 1,
} THEME_CONTROL_DATA;

typedef enum THEME_CONTROL_TYPE
{
    THEME_CONTROL_TYPE_UNKNOWN,
    THEME_CONTROL_TYPE_BILLBOARD,
    THEME_CONTROL_TYPE_BUTTON,
    THEME_CONTROL_TYPE_CHECKBOX,
    THEME_CONTROL_TYPE_COMBOBOX,
    THEME_CONTROL_TYPE_COMMANDLINK,
    THEME_CONTROL_TYPE_EDITBOX,
    THEME_CONTROL_TYPE_HYPERLINK,
    THEME_CONTROL_TYPE_HYPERTEXT,
    THEME_CONTROL_TYPE_IMAGE,
    THEME_CONTROL_TYPE_LABEL,
    THEME_CONTROL_TYPE_PANEL,
    THEME_CONTROL_TYPE_PROGRESSBAR,
    THEME_CONTROL_TYPE_RADIOBUTTON,
    THEME_CONTROL_TYPE_RICHEDIT,
    THEME_CONTROL_TYPE_STATIC,
    THEME_CONTROL_TYPE_LISTVIEW,
    THEME_CONTROL_TYPE_TREEVIEW,
    THEME_CONTROL_TYPE_TAB,
} THEME_CONTROL_TYPE;

typedef enum THEME_SHOW_PAGE_REASON
{
    THEME_SHOW_PAGE_REASON_DEFAULT,
    THEME_SHOW_PAGE_REASON_CANCEL,
    THEME_SHOW_PAGE_REASON_REFRESH,
} THEME_SHOW_PAGE_REASON;

typedef enum THEME_WINDOW_INITIAL_POSITION
{
    THEME_WINDOW_INITIAL_POSITION_DEFAULT,
    THEME_WINDOW_INITIAL_POSITION_CENTER_MONITOR_FROM_COORDINATES,
} THEME_WINDOW_INITIAL_POSITION;


struct THEME_COLUMN
{
    LPWSTR pszName;
    UINT uStringId;
    int nDefaultDpiBaseWidth;
    int nBaseWidth;
    int nWidth;
    BOOL fExpands;
};


struct THEME_TAB
{
    LPWSTR pszName;
    UINT uStringId;
};

struct THEME_ACTION
{
    LPWSTR sczCondition;
    THEME_ACTION_TYPE type;
    union
    {
        struct
        {
            LPWSTR sczVariableName;
        } BrowseDirectory;
        struct
        {
            LPWSTR sczPageName;
            BOOL fCancel;
        } ChangePage;
    };
};

struct THEME_CONDITIONAL_TEXT
{
    LPWSTR sczCondition;
    LPWSTR sczText;
};

// THEME_ASSIGN_CONTROL_ID - Used to apply a specific id to a named control (usually
//                           to set the WM_COMMAND).
struct THEME_ASSIGN_CONTROL_ID
{
    WORD wId;       // id to apply to control
    LPCWSTR wzName; // name of control to match
};

const DWORD THEME_FIRST_ASSIGN_CONTROL_ID = 1024; // Recommended first control id to be assigned.

struct THEME_CONTROL
{
    THEME_CONTROL_TYPE type;

    WORD wId;
    WORD wPageId;

    LPWSTR sczName; // optional name for control, used to apply control id and link the control to a variable.
    LPWSTR sczText;
    LPWSTR sczTooltip;
    LPWSTR sczNote; // optional text for command link
    int nDefaultDpiX;
    int nDefaultDpiY;
    int nDefaultDpiHeight;
    int nDefaultDpiWidth;
    int nX;
    int nY;
    int nHeight;
    int nWidth;
    int nSourceX;
    int nSourceY;
    UINT uStringId;

    LPWSTR sczEnableCondition;
    LPWSTR sczVisibleCondition;
    BOOL fDisableVariableFunctionality;

    HBITMAP hImage;
    HICON hIcon;

    // Don't free these; it's just a handle to the central image lists stored in THEME. The handle is freed once, there.
    HIMAGELIST rghImageList[4];

    DWORD dwStyle;
    DWORD dwExtendedStyle;
    DWORD dwInternalStyle;

    DWORD dwFontId;

    // child controls
    DWORD cControls;
    THEME_CONTROL* rgControls;

    // Used by billboard controls
    WORD wBillboardInterval;
    BOOL fBillboardLoops;

    // Used by button and command link controls
    THEME_ACTION* rgActions;
    DWORD cActions;
    THEME_ACTION* pDefaultAction;

    // Used by hyperlink and owner-drawn button controls
    DWORD dwFontHoverId;
    DWORD dwFontSelectedId;

    // Used by listview controls
    THEME_COLUMN *ptcColumns;
    DWORD cColumns;

    // Used by radio button controls
    BOOL fLastRadioButton;
    LPWSTR sczValue;
    LPWSTR sczVariable;

    // Used by tab controls
    THEME_TAB *pttTabs;
    DWORD cTabs;

    // Used by controls that have text
    DWORD cConditionalText;
    THEME_CONDITIONAL_TEXT* rgConditionalText;

    // Used by command link controls
    DWORD cConditionalNotes;
    THEME_CONDITIONAL_TEXT* rgConditionalNotes;

    // state variables that should be ignored
    HWND hWnd;
    DWORD dwData; // type specific data
};


struct THEME_IMAGELIST
{
    LPWSTR sczName;

    HIMAGELIST hImageList;
};

struct THEME_SAVEDVARIABLE
{
    LPWSTR wzName;
    LPWSTR sczValue;
};

struct THEME_PAGE
{
    WORD wId;
    LPWSTR sczName;

    DWORD cControlIndices;

    DWORD cSavedVariables;
    THEME_SAVEDVARIABLE* rgSavedVariables;
};

struct THEME_FONT_INSTANCE
{
    UINT nDpi;
    HFONT hFont;
};

struct THEME_FONT
{
    LONG lfHeight;
    LONG lfWeight;
    BYTE lfUnderline;
    BYTE lfQuality;
    LPWSTR sczFaceName;

    COLORREF crForeground;
    HBRUSH hForeground;
    COLORREF crBackground;
    HBRUSH hBackground;

    DWORD cFontInstances;
    THEME_FONT_INSTANCE* rgFontInstances;
};


struct THEME
{
    WORD wId;

    BOOL fAutoResize;
    BOOL fForceResize;

    DWORD dwStyle;
    DWORD dwFontId;
    HANDLE hIcon;
    LPWSTR sczCaption;
    int nDefaultDpiHeight;
    int nDefaultDpiMinimumHeight;
    int nDefaultDpiWidth;
    int nDefaultDpiMinimumWidth;
    int nHeight;
    int nMinimumHeight;
    int nWidth;
    int nMinimumWidth;
    int nWindowHeight;
    int nWindowWidth;
    int nSourceX;
    int nSourceY;
    UINT uStringId;

    HBITMAP hImage;

    DWORD cFonts;
    THEME_FONT* rgFonts;

    DWORD cPages;
    THEME_PAGE* rgPages;

    DWORD cImageLists;
    THEME_IMAGELIST* rgImageLists;

    DWORD cControls;
    THEME_CONTROL* rgControls;

    // internal state variables -- do not use outside ThmUtil.cpp
    HWND hwndParent; // parent for loaded controls
    HWND hwndHover; // current hwnd hovered over
    DWORD dwCurrentPageId;
    HWND hwndTooltip;

    UINT nDpi;

    // callback functions
    PFNTHM_EVALUATE_VARIABLE_CONDITION pfnEvaluateCondition;
    PFNTHM_FORMAT_VARIABLE_STRING pfnFormatString;
    PFNTHM_GET_VARIABLE_NUMERIC pfnGetNumericVariable;
    PFNTHM_SET_VARIABLE_NUMERIC pfnSetNumericVariable;
    PFNTHM_GET_VARIABLE_STRING pfnGetStringVariable;
    PFNTHM_SET_VARIABLE_STRING pfnSetStringVariable;

    LPVOID pvVariableContext;
};


/********************************************************************
 ThemeInitialize - initialized theme management.

*******************************************************************/
HRESULT DAPI ThemeInitialize(
    __in_opt HMODULE hModule
    );

/********************************************************************
 ThemeUninitialize - uninitialize theme management.

*******************************************************************/
void DAPI ThemeUninitialize();

/********************************************************************
 ThemeLoadFromFile - loads a theme from a loose file.

 *******************************************************************/
HRESULT DAPI ThemeLoadFromFile(
    __in_z LPCWSTR wzThemeFile,
    __out THEME** ppTheme
    );

/********************************************************************
 ThemeLoadFromResource - loads a theme from a module's data resource.

 NOTE: The resource data must be UTF-8 encoded.
*******************************************************************/
HRESULT DAPI ThemeLoadFromResource(
    __in_opt HMODULE hModule,
    __in_z LPCSTR szResource,
    __out THEME** ppTheme
    );

/********************************************************************
 ThemeFree - frees any memory associated with a theme.

*******************************************************************/
void DAPI ThemeFree(
    __in THEME* pTheme
    );

/********************************************************************
ThemeRegisterVariableCallbacks - registers a context and callbacks
                                 for working with variables.

*******************************************************************/
HRESULT DAPI ThemeRegisterVariableCallbacks(
    __in THEME* pTheme,
    __in_opt PFNTHM_EVALUATE_VARIABLE_CONDITION pfnEvaluateCondition,
    __in_opt PFNTHM_FORMAT_VARIABLE_STRING pfnFormatString,
    __in_opt PFNTHM_GET_VARIABLE_NUMERIC pfnGetNumericVariable,
    __in_opt PFNTHM_SET_VARIABLE_NUMERIC pfnSetNumericVariable,
    __in_opt PFNTHM_GET_VARIABLE_STRING pfnGetStringVariable,
    __in_opt PFNTHM_SET_VARIABLE_STRING pfnSetStringVariable,
    __in_opt LPVOID pvContext
    );

/********************************************************************
 ThemeCreateParentWindow - creates a parent window for the theme.

*******************************************************************/
HRESULT DAPI ThemeCreateParentWindow(
    __in THEME* pTheme,
    __in DWORD dwExStyle,
    __in LPCWSTR szClassName,
    __in LPCWSTR szWindowName,
    __in DWORD dwStyle,
    __in int x,
    __in int y,
    __in_opt HWND hwndParent,
    __in_opt HINSTANCE hInstance,
    __in_opt LPVOID lpParam,
    __in THEME_WINDOW_INITIAL_POSITION initialPosition,
    __out_opt HWND* phWnd
    );

/********************************************************************
 ThemeLoadControls - creates the windows for all the theme controls
                     using the window created in ThemeCreateParentWindow.

*******************************************************************/
HRESULT DAPI ThemeLoadControls(
    __in THEME* pTheme,
    __in_ecount_opt(cAssignControlIds) const THEME_ASSIGN_CONTROL_ID* rgAssignControlIds,
    __in DWORD cAssignControlIds
    );

/********************************************************************
 ThemeUnloadControls - resets all the theme control windows so the theme
                       controls can be reloaded.

*******************************************************************/
void DAPI ThemeUnloadControls(
    __in THEME* pTheme
    );

/********************************************************************
 ThemeLocalize - Localizes all of the strings in the theme.

*******************************************************************/
HRESULT DAPI ThemeLocalize(
    __in THEME *pTheme,
    __in const WIX_LOCALIZATION *pLocStringSet
    );

HRESULT DAPI ThemeLoadStrings(
    __in THEME* pTheme,
    __in HMODULE hResModule
    );

/********************************************************************
 ThemeLoadRichEditFromFile - Attach a richedit control to a RTF file.

 *******************************************************************/
HRESULT DAPI ThemeLoadRichEditFromFile(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in_z LPCWSTR wzFileName,
    __in HMODULE hModule
    );

/********************************************************************
 ThemeLoadRichEditFromResource - Attach a richedit control to resource data.

 *******************************************************************/
HRESULT DAPI ThemeLoadRichEditFromResource(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in_z LPCSTR szResourceName,
    __in HMODULE hModule
    );

/********************************************************************
 ThemeLoadRichEditFromResourceToHWnd - Attach a richedit control (by 
                                       HWND) to resource data.

 *******************************************************************/
HRESULT DAPI ThemeLoadRichEditFromResourceToHWnd(
    __in HWND hWnd,
    __in_z LPCSTR szResourceName,
    __in HMODULE hModule
    );

/********************************************************************
 ThemeHandleKeyboardMessage - will translate the message using the active
                             accelerator table.

*******************************************************************/
BOOL DAPI ThemeHandleKeyboardMessage(
    __in_opt THEME* pTheme,
    __in HWND hWnd,
    __in MSG* pMsg
    );

/********************************************************************
 ThemeDefWindowProc - replacement for DefWindowProc() when using theme.

*******************************************************************/
LRESULT CALLBACK ThemeDefWindowProc(
    __in_opt THEME* pTheme,
    __in HWND hWnd,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );

/********************************************************************
 ThemeGetPageIds - gets the page ids for the theme via page names.

*******************************************************************/
void DAPI ThemeGetPageIds(
    __in const THEME* pTheme,
    __in_ecount(cGetPages) LPCWSTR* rgwzFindNames,
    __inout_ecount(cGetPages) DWORD* rgdwPageIds,
    __in DWORD cGetPages
    );

/********************************************************************
 ThemeGetPage - gets a theme page by id.

 *******************************************************************/
THEME_PAGE* DAPI ThemeGetPage(
    __in const THEME* pTheme,
    __in DWORD dwPage
    );

/********************************************************************
 ThemeShowPage - shows or hides all of the controls in the page at one time.

 *******************************************************************/
HRESULT DAPI ThemeShowPage(
    __in THEME* pTheme,
    __in DWORD dwPage,
    __in int nCmdShow
    );

/********************************************************************
ThemeShowPageEx - shows or hides all of the controls in the page at one time.
                  When using variables, TSPR_CANCEL reverts any changes made.
                  TSPR_REFRESH forces reevaluation of conditions.
                  It is expected that the current page is hidden before 
                  showing a new page.

*******************************************************************/
HRESULT DAPI ThemeShowPageEx(
    __in THEME* pTheme,
    __in DWORD dwPage,
    __in int nCmdShow,
    __in THEME_SHOW_PAGE_REASON reason
    );


/********************************************************************
ThemeShowChild - shows a control's specified child control, hiding the rest.

*******************************************************************/
void DAPI ThemeShowChild(
    __in THEME* pTheme,
    __in THEME_CONTROL* pParentControl,
    __in DWORD dwIndex
    );

/********************************************************************
 ThemeControlExists - check if a control with the specified id exists.

 *******************************************************************/
BOOL DAPI ThemeControlExists(
    __in const THEME* pTheme,
    __in DWORD dwControl
    );

/********************************************************************
 ThemeControlEnable - enables/disables a control.

 *******************************************************************/
void DAPI ThemeControlEnable(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in BOOL fEnable
    );

/********************************************************************
 ThemeControlEnabled - returns whether a control is enabled/disabled.

 *******************************************************************/
BOOL DAPI ThemeControlEnabled(
    __in THEME* pTheme,
    __in DWORD dwControl
    );

/********************************************************************
 ThemeControlElevates - sets/removes the shield icon on a control.

 *******************************************************************/
void DAPI ThemeControlElevates(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in BOOL fElevates
    );

/********************************************************************
 ThemeShowControl - shows/hides a control.

 *******************************************************************/
void DAPI ThemeShowControl(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in int nCmdShow
    );

/********************************************************************
ThemeShowControlEx - shows/hides a control with support for 
conditional text and notes.

*******************************************************************/
void DAPI ThemeShowControlEx(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in int nCmdShow
    );

/********************************************************************
 ThemeControlVisible - returns whether a control is visible.

 *******************************************************************/
BOOL DAPI ThemeControlVisible(
    __in THEME* pTheme,
    __in DWORD dwControl
    );

BOOL DAPI ThemePostControlMessage(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in UINT Msg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );

LRESULT DAPI ThemeSendControlMessage(
    __in const THEME* pTheme,
    __in DWORD dwControl,
    __in UINT Msg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );

/********************************************************************
 ThemeDrawBackground - draws the theme background.

*******************************************************************/
HRESULT DAPI ThemeDrawBackground(
    __in THEME* pTheme,
    __in PAINTSTRUCT* pps
    );

/********************************************************************
 ThemeDrawControl - draw an owner drawn control.

*******************************************************************/
HRESULT DAPI ThemeDrawControl(
    __in THEME* pTheme,
    __in DRAWITEMSTRUCT* pdis
    );

/********************************************************************
 ThemeHoverControl - mark a control as hover.

*******************************************************************/
BOOL DAPI ThemeHoverControl(
    __in THEME* pTheme,
    __in HWND hwndParent,
    __in HWND hwndControl
    );

/********************************************************************
 ThemeIsControlChecked - gets whether a control is checked. Only
                         really useful for checkbox controls.

*******************************************************************/
BOOL DAPI ThemeIsControlChecked(
    __in THEME* pTheme,
    __in DWORD dwControl
    );

/********************************************************************
 ThemeSetControlColor - sets the color of text for a control.

*******************************************************************/
BOOL DAPI ThemeSetControlColor(
    __in THEME* pTheme,
    __in HDC hdc,
    __in HWND hWnd,
    __out HBRUSH* phBackgroundBrush
    );

/********************************************************************
 ThemeSetProgressControl - sets the current percentage complete in a
                           progress bar control.

*******************************************************************/
HRESULT DAPI ThemeSetProgressControl(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in DWORD dwProgressPercentage
    );

/********************************************************************
 ThemeSetProgressControlColor - sets the current color of a
                                progress bar control.

*******************************************************************/
HRESULT DAPI ThemeSetProgressControlColor(
    __in THEME* pTheme,
    __in DWORD dwControl,
    __in DWORD dwColorIndex
    );

/********************************************************************
 ThemeSetTextControl - sets the text of a control.

*******************************************************************/
HRESULT DAPI ThemeSetTextControl(
    __in const THEME* pTheme,
    __in DWORD dwControl,
    __in_z_opt LPCWSTR wzText
    );

/********************************************************************
ThemeSetTextControl - sets the text of a control and optionally
                      invalidates the control.

*******************************************************************/
HRESULT DAPI ThemeSetTextControlEx(
    __in const THEME* pTheme,
    __in DWORD dwControl,
    __in BOOL fUpdate,
    __in_z_opt LPCWSTR wzText
    );

/********************************************************************
 ThemeGetTextControl - gets the text of a control.

*******************************************************************/
HRESULT DAPI ThemeGetTextControl(
    __in const THEME* pTheme,
    __in DWORD dwControl,
    __inout_z LPWSTR* psczText
    );

/********************************************************************
 ThemeUpdateCaption - updates the caption in the theme.

*******************************************************************/
HRESULT DAPI ThemeUpdateCaption(
    __in THEME* pTheme,
    __in_z LPCWSTR wzCaption
    );

/********************************************************************
 ThemeSetFocus - set the focus to the control supplied or the next 
                 enabled control if it is disabled.

*******************************************************************/
void DAPI ThemeSetFocus(
    __in THEME* pTheme,
    __in DWORD dwControl
    );

#ifdef __cplusplus
}
#endif

