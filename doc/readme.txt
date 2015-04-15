CW2FMIDriver
  * CW2FMIGenerator.exe - generator PAR souboru z FMU.
  * CW2FMIDriver.dll - driver pro ControlWeb verze 3
  * fmilib_shared.dll - obecny driver pro FMI

=== Postup pouziti ===
  1. rozbal zip do adresare napr. C:\CW2FMIHummod
 
  2. vygeneruj FMU pro Co-Simulaci z Modelicoveho nastroje a nastavit ImplementationTag na true.
     v Dymole 
     Advanced.FMI.IncludeImplementationTag = true; 

         menu -> Simulation -> Translate -> FMU
     translateModelFMU("HumMod.HumMod_GolemEdition", false);
 
  3. zkopiruj FMU do adresare z bodu 1.

  4. vygeneruj PAR soubor pro FMU
     spust command line: cmd
     cd do adresare z bodu 1: napr. 
     
     cd c:\CW2FMIHummod

     spust CW2FMIGenerator.exe s parametry 1. nazev FMU 
souboru a 2.nazev noveho souboru s priponou par

     CW2FMIGenerator.exe HumMod_Hummod_GolemEdition.fmu hummod.par
 
  5. kvuli bug #107, zkopiruj DLL z podadresare tempfmu/binaries/win32/ do systemoveho adresare napr. c:\windows\system32

  6. spust ControlWeb, nainstaluj driver, nahraj PAR soubor vygenerovany v kroku 4. a DMF soubor
vytvoreny rucne
  
  7. kvuli bug #xxx zkontroluj cestu, ve ktere pracuje CW, pripadne zmenit v nabidce start.


=== ridici volani z ControlWebu do FMI ===

Predpoklad:

driver
  cw2fmi {driver = 'cw2fmidriver.dll'; map_file = 'Hummod.dmf'; parameter_file = 'Hummod.par'};
end_driver;
...    

  system.DriverQueryProc( 'cw2fmi', 'start', &lc );
nastartuje simulaci od posledniho kroku (pokud simulace dosahne definovaneho casu konce) pak se sama zastavi
Krok simulace se provede az kdyz si pomoci kanalu vyzada ovladac aspon jednu hodnotu nejake promenne

  system.DriverQueryProc( 'cw2fmi', 'stop', &lc );
Pozastavi simulaci, dotaz na data z kanalu vraci posledni hodnotu ze simulace

  system.DriverQueryProc( 'cw2fmi', step , &lc );
v step je integer, pocet kroku, ktere simulator vykona, pokud je pozastaven, opet se zastavi

  system.DriverQueryProc( 'cw2fmi', 'starttime', &lr );
v &lr je hodnota long real. Nastavi startovni cas simulace. Nutne zavolat 'init'.

  system.DriverQueryProc( 'cw2fmi', 'stoptime', &lr );
v &lr je hodnota long real. Nastavi koncovy cas simulace. Nutne zavolat 'init'.

  system.DriverQueryProc( 'cw2fmi', 'steptime', &lr );
v &lr je hodnota long real. Nastavi krok simulace. Nutne zavolat 'init'.

  system.DriverQueryProc( 'cw2fmi', 'init', &lc );
Inicializuje simulator nastavi pocatecni a koncovy cas a krok simulace nastavene v predchozich
volanich starttime, stoptime a steptime. Po tomto volani je mozne nektere vstupy a parametry modelu menit
pres kanaly.

=== version 15.04 ===
  * updated CW@FMIDriver.dll, opravy nekterych simulacnich chyb a detekce adresare pri inicializaci 

=== version 1.03 ===
  * update CW2FMIDriver.dll pridana moznost nastavit starttime na jakoukoliv hodnotu, odsimuluje se od 0 skryte, pokud steptime =0, pak se
neprovede simulace ale jen se mohou nacist a nastavit hodnoty parametru a promennych

=== version 1.02 ===
  * update CW2FMIDriver.dll a CW2FMIGenerator.exe, release verze nezavisi na debug verzich 
obecnych knihoven visual studia, ktere nemusi byt nainstalovany na vsech platformach s Windows.

=== version 1.01 ===
  * update CW2FMIDriver.dll, pridany ridici prikazy pomoci DriverQueryProc a popis. Opraven text v CW2FMIGenerator.exe

=== version 1.0 ===
  * soubory: CW2FMIDriver.dll, fmilib_shared.dll, CW2FMIGenerator.exe
  * test: test/*.* 
  * zname problemy:
    * bug #107 CW2FMI driver nenahraje DLL z prilozeneho FMU.
Workaround: Nahrat DLL z FMU do systemoveho adresare napr c:\Windows\System32
    * bug #108 CW spadne na StackOverFlow pri nahravani velkeho modelu - napr. Hummod.FMU
Workaround: Zmenit STACK v binarce CW6.EXE pomoci utility EDITBIN napr na 10MB.
Spustit Visual Studio Command Prompt (Start->Microsoft Visual Studio -> Tools > Visual Studio Command Prompt)
EDITBIN /STACK:10485760 "path to cw6.exe"
    * bug #xxx CW nastartuje z domovskeho adresare, nezapisou se logy a vyhodi se jen hlaska your FMU version is 0, dir:C:\Program Files ...



