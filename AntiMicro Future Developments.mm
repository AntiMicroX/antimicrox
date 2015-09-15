<map version="1.0.1">
<!-- To view this file, download free mind mapping software FreeMind from http://freemind.sourceforge.net -->
<node CREATED="1379712845116" ID="ID_571348173" MODIFIED="1420094589396" TEXT="AntiMicro Future Developments">
<node CREATED="1398676376577" FOLDED="true" HGAP="19" ID="ID_1223827762" MODIFIED="1427231988304" POSITION="left" TEXT="Split Actions into a Seperate Entity?" VSHIFT="-5">
<icon BUILTIN="help"/>
<node CREATED="1398676422304" ID="ID_744925725" MODIFIED="1398676855592" TEXT="Unlike the current system where assignment slots are assigned to controller buttons, actions would hold all applicable slots and a button would then be assigned an action. The major benefit behind doing that would be that complex macro assignments can be assigned to multiple buttons without completely mapping the sequence out again. Also, it would be much easier to assign the sequence to a different button.&#xa;&#xa;This type of change would probably require a complete program refactor. I honestly don&apos;t see this feature ever being incorporated into AntiMicro."/>
</node>
<node CREATED="1398677269018" HGAP="14" ID="ID_1391547769" MODIFIED="1398677391581" POSITION="left" TEXT="Wizards for Assigning More Complex Actions" VSHIFT="-7"/>
<node CREATED="1420094431903" FOLDED="true" HGAP="18" ID="ID_475045149" MODIFIED="1420094854839" POSITION="right" TEXT="Allow buttons to be bound to actions." VSHIFT="-4">
<node CREATED="1420094839855" ID="ID_1670829131" MODIFIED="1420094853324" TEXT="A special exception will have to be made for direct assignments."/>
</node>
<node CREATED="1420094449038" FOLDED="true" HGAP="22" ID="ID_287590243" MODIFIED="1433647333360" POSITION="right" TEXT="Use uinput by default and fallback to XTest if necessary." VSHIFT="-5">
<icon BUILTIN="button_ok"/>
<node CREATED="1420094775157" ID="ID_1731182902" MODIFIED="1420094816470" TEXT="Most of the work has already been done. At this point, it is just a matter of making uinput the default when no handler is given as a command line argument."/>
</node>
<node CREATED="1420094462889" FOLDED="true" ID="ID_1831540910" MODIFIED="1442328189165" POSITION="right" TEXT="Move simulated event generation to a new thread.">
<icon BUILTIN="button_ok"/>
<node CREATED="1420094736282" ID="ID_826806136" MODIFIED="1420094766897" TEXT="This will mainly help people on Windows but it will allow timers within the program to be more precise."/>
</node>
<node CREATED="1420094472645" FOLDED="true" ID="ID_1669687536" MODIFIED="1427231960129" POSITION="left" TEXT="Allow logging as long as it doesn&apos;t cause button lag.">
<icon BUILTIN="button_ok"/>
<node CREATED="1420094861410" ID="ID_1107261160" MODIFIED="1420094909890" TEXT="If a default logging scheme causes button lag then the idea will be scrapped."/>
<node CREATED="1420094929178" ID="ID_1780841030" MODIFIED="1420095026698" TEXT="There doesn&apos;t seem to be a good, lean logging library for Qt that can be used in Release builds. There are a few libraries that rely on qDebug which won&apos;t help for Release builds." VSHIFT="14"/>
<node CREATED="1427231598964" HGAP="19" ID="ID_1669264888" MODIFIED="1427231683161" TEXT="Queueing messages and writing to streams after processing button presses helped. That seems like the best compromise in order to prevent adding lag to button presses." VSHIFT="20"/>
</node>
<node CREATED="1420094539813" FOLDED="true" HGAP="17" ID="ID_1973751052" MODIFIED="1420094731616" POSITION="right" TEXT="Allow notes to be added to a profile in various places." VSHIFT="6">
<node CREATED="1420094662812" ID="ID_473219128" MODIFIED="1420094730599" TEXT="Sometimes, a user will need to add extra notes in order to be able to fully explain to other people what an action does. Extra notes would also be useful at the profile level if special changes need to be made in a game in order for the profile to work as intended."/>
</node>
</node>
</map>
