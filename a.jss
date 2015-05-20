include  "hjconst.jsh";

Script AltF8 ()
TypeCurrentScriptKey();
SayLine();
EndScript

Script ControlDownArrow ()
TypeCurrentScriptKey();
SayLine();
EndScript

Script ControlUpArrow ()
TypeCurrentScriptKey();
SayLine();
EndScript

Script AltLeftArrow ()
TypeCurrentScriptKey();
SayLine();
EndScript

Script AltRightArrow ()
TypeCurrentScriptKey();
SayLine();
EndScript

Script MouseRight ()
TypeCurrentScriptKey();
performscript SaySelectedText();
EndScript

Script MouseLeft ()
TypeCurrentScriptKey();
performscript SaySelectedText();
EndScript

Script MouseUp ()
TypeCurrentScriptKey();
performscript SaySelectedText();
EndScript

Script MouseDown ()
TypeCurrentScriptKey();
performscript SaySelectedText();
EndScript

Script OpenListBox ()
if GetObjectTypeCode() == WT_EDIT then
TypeCurrentScriptKey()
SayLine();
Else
PerformScript OpenListBox();
EndIf
EndScript

Script CloseListBox ()
if GetObjectTypeCode() == WT_EDIT then
TypeCurrentScriptKey()
SayLine();
Else
PerformScript CloseListBox();
EndIf
EndScript

Script UpCell  ()
if GetObjectTypeCode() == WT_EDIT then
TypeCurrentScriptKey()
SayLine();
Else
PerformScript UpCell();
EndIf
EndScript

Script DownCell  ()
if GetObjectTypeCode() == WT_EDIT then
TypeCurrentScriptKey()
SayLine();
Else
PerformScript DownCell();
EndIf
EndScript

Script NextCell  ()
if GetObjectTypeCode() == WT_EDIT then
TypeCurrentScriptKey()
SayLine();
Else
PerformScript NextCell();
EndIf
EndScript

Script PriorCell  ()
if GetObjectTypeCode() == WT_EDIT then
TypeCurrentScriptKey()
SayLine();
Else
PerformScript PriorCell();
EndIf
EndScript
