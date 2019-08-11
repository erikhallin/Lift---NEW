Lift_leveleditor.exe

Läser filen level.bmp och hittar alla svarta pixlar och gör en lista av dess koordinater.
Mellan dessa koordinatpunkter drags sträck, vilket kommer att vara terrängmarken på banan.
Programmet kommer att dra en linje från punkten längst till vänster till närmsta punkt.
Och sedan från denna punkt dra en linje till närmsta nästa punkt.

Vid start väljs ett värde "Cut-off" för hur långt avstånd mellan 2 punkter som inte ska bli
ihopkopplade. Behövs för att göra fria strukturer som inte är kopplade till bottenlagret.
Detta kommer att skriva in "BREAK" mellan två punkter i "level.txt".
Dessa fria strukturer kommer att länkas från första punkt till sista. Den enda struktur som inte
kommer att länkas är den första som hittas, vilket kommer att representera bottenlagret.

När Lift_leveleditor.exe är klart kommer den att skriva en level.txt fil med dessa koordinater.
Var startpositionen för spelaren ska vara kan väljas genom att ändra koordinaten för "playerpos".
För att ändra storleken på banan kan "scale_factor" ändras från 1 1 till andra värden för att
multiplicera kordinaterna med respektive faktor.



EXTRA?
lägg in ljud i kod?

fler banor?

fler terrain textures?

fiende motor ljud?
mship auto landing complete?
mship takeoff begin/canceled?
col ljud i meny?
col ljud för obj?


BUGGAR?

krash när textruta visades samtidigt som ship i input box (skepp dockat till scanner ner i mship först)

bugg krash efter ship upgrade efter pant av vapen...

KANSKE för att worldptr i weapon ej updaterades vid världsbyte, fixad nu

vid fire..1234 (när ny kropp skapas) krash
händer när projektil skapas
kanske när den skapas inuti annan kropp
krash även vid 9 och senare
terrain försvan precis innan? ritades ej

när rep fälldes ut, låg object i inputbox

krash vid fire, på 9, spakade fixture, kanske inuti annan kropp och skapade collision? tags bort?



Förslag

?börja ryka om trasig?
?ta bort cam gear?
?fiender ska kunna ändra target, tex om tar skada?
?static lib för oggvorbis?
?planet names?
?fiender spawnas utanför terräng?
?gradient skyline? ljusare nära ytan?
