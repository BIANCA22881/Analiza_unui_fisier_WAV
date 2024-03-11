/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: onMainPanel */
#define  PANEL_GRAPH                      2       /* control type: graph, callback function: (none) */
#define  PANEL_GRAPH_2                    3       /* control type: graph, callback function: (none) */
#define  PANEL_COMMANDBUTTON              4       /* control type: command, callback function: onExit */
#define  PANEL_COMMANDBUTTON_2            5       /* control type: command, callback function: OnLoadButtonCB */
#define  PANEL_COMMANDBUTTON_8            6       /* control type: command, callback function: onKurtosis */
#define  PANEL_COMMANDBUTTON_10           7       /* control type: command, callback function: onAnvelopa */
#define  PANEL_COMMANDBUTTON_7            8       /* control type: command, callback function: onSkewness */
#define  PANEL_COMMANDBUTTON_5            9       /* control type: command, callback function: onAplica */
#define  PANEL_COMMANDBUTTON_4            10      /* control type: command, callback function: onNext */
#define  PANEL_COMMANDBUTTON_6            11      /* control type: command, callback function: onReset */
#define  PANEL_COMMANDBUTTON_3            12      /* control type: command, callback function: onPrev */
#define  PANEL_NUMERIC_12                 13      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_11                 14      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_2                  15      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC                    16      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_8                  17      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_10                 18      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_9                  19      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_7                  20      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_6                  21      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_5                  22      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_4                  23      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_3                  24      /* control type: numeric, callback function: (none) */
#define  PANEL_RING                       25      /* control type: ring, callback function: (none) */
#define  PANEL_GRAPH_3                    26      /* control type: graph, callback function: (none) */
#define  PANEL_RINGSLIDE                  27      /* control type: slide, callback function: (none) */
#define  PANEL_TEXTMSG                    28      /* control type: textMsg, callback function: (none) */
#define  PANEL_DECORATION                 29      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_2               30      /* control type: deco, callback function: (none) */
#define  PANEL_DECORATION_3               31      /* control type: deco, callback function: (none) */
#define  PANEL_TEXTMSG_2                  32      /* control type: textMsg, callback function: (none) */
#define  PANEL_TEXTMSG_3                  33      /* control type: textMsg, callback function: (none) */
#define  PANEL_DECORATION_4               34      /* control type: deco, callback function: (none) */
#define  PANEL_GRAPH_4                    35      /* control type: graph, callback function: (none) */
#define  PANEL_BINARYSWITCH               36      /* control type: binary, callback function: onSwitchPanels */
#define  PANEL_DECORATION_5               37      /* control type: deco, callback function: (none) */

#define  PANEL_2                          2       /* callback function: onFrequencyPanel */
#define  PANEL_2_BINARYSWITCH             2       /* control type: binary, callback function: onSwitchPanels */
#define  PANEL_2_GRAPH_3                  3       /* control type: graph, callback function: (none) */
#define  PANEL_2_GRAPH_4                  4       /* control type: graph, callback function: (none) */
#define  PANEL_2_GRAPH_2                  5       /* control type: graph, callback function: (none) */
#define  PANEL_2_GRAPH                    6       /* control type: graph, callback function: (none) */
#define  PANEL_2_RINGSLIDE                7       /* control type: slide, callback function: (none) */
#define  PANEL_2_TOGGLEBUTTON_2           8       /* control type: textButton, callback function: onStartSpectruFiltrat */
#define  PANEL_2_TOGGLEBUTTON             9       /* control type: textButton, callback function: onLoadSpectrum */
#define  PANEL_2_RING_2                   10      /* control type: ring, callback function: (none) */
#define  PANEL_2_RING                     11      /* control type: ring, callback function: (none) */
#define  PANEL_2_NUMERIC_3                12      /* control type: numeric, callback function: (none) */
#define  PANEL_2_NUMERIC_4                13      /* control type: numeric, callback function: (none) */
#define  PANEL_2_NUMERIC_2                14      /* control type: numeric, callback function: (none) */
#define  PANEL_2_NUMERIC                  15      /* control type: numeric, callback function: (none) */
#define  PANEL_2_TIMER                    16      /* control type: timer, callback function: onTick */
#define  PANEL_2_PICTURE                  17      /* control type: picture, callback function: (none) */
#define  PANEL_2_COMMANDBUTTON            18      /* control type: command, callback function: onTakePicture */


     /* Control Arrays: */

          /* (no control arrays in the resource file) */


     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK onAnvelopa(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onAplica(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onExit(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onFrequencyPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onKurtosis(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLoadButtonCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onLoadSpectrum(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onMainPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onNext(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onPrev(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onReset(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onSkewness(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onStartSpectruFiltrat(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onSwitchPanels(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onTakePicture(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK onTick(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif