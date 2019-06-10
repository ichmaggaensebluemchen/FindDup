/* Matrikelnummer: 2125287*/

/*C-Bibliotheken und Header-Dateien*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys\stat.h>
#include <time.h>

/*Makros*/								//für einheitliche Arraygößen
#define GROESSE 255						
#define TYPGROESSE 6
#define DATGROESSE 30			

/*anlegen einer Datei-Struktur*/
	typedef struct datei_struktur
	{
		char dateiname[GROESSE];
		char datum[DATGROESSE];
		char typ[TYPGROESSE];
		unsigned long dateiGroesse;
		
		struct datei_struktur *next;
	} datei;

/*globale Struktur für potenziell doppelte Dateien*/
	struct paare
	{
		char nameDatEins[GROESSE];
		char nameDatZwei[GROESSE];
		char datumDatEins[DATGROESSE];
		char datumDatZwei[DATGROESSE];
		char typDatEins[TYPGROESSE];
		char typDatZwei[TYPGROESSE];
		unsigned long grDatEins;
		unsigned long grDatZwei;
	} duplikate[100];
	
/*Funktionsprototypen*/
void benutzereingabe();									//Funktion zur Eingabe der benötigten Informationen
void defaultVorschlag();								//Funktion für einen Vorschlag für Pfad und Namen der Ausgabedatei
int dateiExtTxt(char *);								//Funktion zur Feststellung ob ein Dateineme mit ".txt" endet
void zieldateiOk(char *);								//Funktion zur Prüfung ob es sich bei der Ausgabedatei, um eine Datei handelt
void attributeAuslesen();					            //Funktion für das Anlegen zweier Dateilisten und das Auslesen der Dateiattribute
void vergleich(struct paare *, datei *, datei *);		//Funktion für die Suche von wahrscheinlich gleichen Dateien
void zielDateiSchreiben(struct paare *);				//Funktion für die Erstellung der Zieldatei, die Ergebnisse enthält
void ausgabe(struct paare *);							//Funktion für die Bildschirmausgabe der Ergebnisse

/*globale Variablen*/
char verzeichnisEins[GROESSE];							//erstes Vergleichsverzeichnis
char verzeichnisZwei[GROESSE];							//zweites Vergleichsverzeichnis
char zielDatei[GROESSE];								//Verzeichnis und Dateiname des Ergebnisliste
char unterverzeichnisse[GROESSE];						//Einbezug der Unterverzeichnisse oder nicht

int dup_index = 0;										//Index für das Array, das Duplikate speichert

/*Hauptprogramm zum Programm finddup*/
int main(int argc, char *argv[]) 
{
	/*Variablendeklaration*/
	DIR *p_verzeichinsEins;
	DIR *p_verzeichinsZwei;
	FILE *file_zielDat;
	const char *finden;
	char dateiExt[TYPGROESSE];
	int anzahl = 0;
	
	/*Kopfbereich des Programms*/
	printf("Finddup - Doppelte Dateien finden\n");
	printf("Matrikelnummer: 2125287\n\n\n");
	
	/*Differenzierung der zwei Eingabemöglichkeiten*/
	if (argc == 5)										//es existieren die vier benötigten Übergabeparameter
	{
		/*Prüfung, ob angegebene Verzeichnisse existieren*/
		if(((p_verzeichinsEins = opendir(argv[1])) != NULL) && ((p_verzeichinsZwei = opendir(argv[2])) != NULL))
		{
			strcpy(verzeichnisEins, argv[1]);
			closedir (p_verzeichinsEins);
			strcpy(verzeichnisZwei, argv[2]);
			closedir (p_verzeichinsZwei);
		}
		else											//ausweichen auf Benutzereingabe
		{
			benutzereingabe();
		}
		
		/*Prüfung, ob Zieldatei erstellt werden kann*/
		zieldateiOk(argv[3]);

		/*Prüfung, ob Unterverzeichniswunsch korrekt angegeben wurde*/
		if((!strcmp(argv[4], "Ja")) || (!strcmp(argv[4], "Nein")))
		{
			strcpy(unterverzeichnisse, argv[4]);
		}
		
		else											//ausweichen auf Benutzereingabe
		{
			benutzereingabe();
		}
	}
	else												//Benutzereingabe, da keine Parameter an das Programm übergeben wurden
	{
		benutzereingabe();
	}
	
	/*Funktionsaufrufe zur Verarbeitung der Eingaben*/
	attributeAuslesen();
	zielDateiSchreiben(duplikate);
	ausgabe(duplikate);
	printf("Das Ergebnis des Vergleichs wurde in der Zieldatei unter %s gespeichert!\n\n", zielDatei);
	getch();
	return 0;
}

void benutzereingabe()									//Schrittweise Eingabe der erforderlichen Informationen für den Vergleich
{
	/*Variable zum Test, ob sich die Verzeichnisse öffnen lassen*/
	DIR *p_verzeichnis;
	/*Variable zum Test, ob sich die Datei öffnen lässt*/
	FILE *p_datei;
	
	printf("Im folgenden werden Sie aufgefordert, die erforderlichen Informationen f%cr den Vergleich, einzugeben.\nEs werden zwei auf Ihrem PC vorhandene Verzeichnisse mit vollst%cndigen Pfadangaben, \nsowie ein Verzeichnis mit einem Dateinamen abgefragt.\n\n", 129, 132);
	printf("Bitte geben Sie den Pfad des ersten Verzeichnisses ein (zum Beispiel: C:\\Verzeichnis1): ");
	gets(verzeichnisEins);
	while(!(p_verzeichnis = opendir(verzeichnisEins)))
	{
		printf("Das Verzeichnis konnte nicht ge%cffnet werden. \nBitte geben Sie erneut einen Pfad f%cr das erste Verzeichnis ein (zum Beispiel: C:\\Verzeichnis1): ", 148 ,129);
		gets(verzeichnisEins);
		closedir(p_verzeichnis);
	}
	
	printf("Bitte geben Sie den Pfad des zweiten Verzeichnisses ein (zum Beispiel: C:\\Verzeichnis2): ");
	gets(verzeichnisZwei);
	while(!(p_verzeichnis = opendir(verzeichnisZwei)))
	{
		printf("Das Verzeichnis konnte nicht ge%cffnet werden. \nBitte geben Sie erneut einen Pfad f%cr das zweite Verzeichnis ein (zum Beispiel: C:\\Verzeichnis2): ", 148 ,129);
		gets(verzeichnisZwei);
		closedir(p_verzeichnis);
	}
	
	while((p_datei = fopen(zielDatei, "w")) == 0)
	{
		printf("Bitte geben Sie das Verzeichnis, sowie den Dateinamen der Zieldatei ein, \ndie das Ergebnis des Vergleichs speichert (zum Beispiel: C:\\Ergebnis.txt): ");
		gets(zielDatei);		
		zieldateiOk(zielDatei); 						//Prüfung, ob Zieldatei erstellt werden kann (Aufruf der Funktion)
		fclose(p_datei);
	}
	
	printf("Wollen Sie die Unterverzeichnisse Ihrer Suchverzeichnisse f%cr den Vergleich ber%ccksichtigen?\nGeben Sie 'Ja' oder 'Nein' ein: ", 129, 129);
	gets(unterverzeichnisse);
	while((strcmp(unterverzeichnisse, "Ja")) & (strcmp(unterverzeichnisse, "Nein")))
	{
		printf("Ihre Eingabe war nicht korrekt! Geben Sie 'Ja'ein, wenn Sie die Unterverzeichnisse einbinden wollen.\nWenn nicht, geben Sie 'Nein'  ein: ", 129, 129);
		gets(unterverzeichnisse);
	}
}

void zieldateiOk(char *eingabeFileName)
{
	FILE *file_zielDat;
	if(strrchr(eingabeFileName, '.') != NULL)	//Kommt in der Eingabe ein '.' vor, wenn nicht ist es keine Datei	
		{
			if (dateiExtTxt(eingabeFileName))	//Hat die Datei die Endung ".txt"
			{
				//es handelt sich um einen Dateinamen mit Endung .txt
				file_zielDat = fopen(eingabeFileName, "w");
				if(file_zielDat)		// Konnte die Datei geöffnet werden
				{
					//es ist möglich die Zeildatei zu erstellen
					strcpy(zielDatei, eingabeFileName);
				}
				else
				{
					//Die Datei konnnte nicht geöffnet werden -> default-Vorschlag
					defaultVorschlag();
				}
				fclose(file_zielDat);
			}
			else
			{
				// es handelt sich um keine .txt Datei - > default-Vorschlag
				defaultVorschlag();
			}
		
		}
		else
		{
			//default-Vorschlag für Zieldatei
			defaultVorschlag();
		}
}

void defaultVorschlag()					//Funktion erstellt einen default-Vorschlag für Zieldatei und fragt nach Einverständnis des Benutzers
{	
	char eingabe[10];
	char vorschlag[GROESSE];
	FILE *p_datei;
	
	system("cls");
	printf("Finddup - Doppelte Dateien finden\n");
	printf("Matrikelnummer: 2125287\n\n");
	strcpy(vorschlag, verzeichnisEins);
	strcat(vorschlag, "\\Ergebnis.txt");
	printf("Die Angabe zur Zieldatei ist nicht korrekt. Soll die Datei unter %s erstellt werden? \nBitte geben Sie 'Ja' oder 'Nein' ein! ", vorschlag);
	gets(eingabe);
	if(!(strcmp(eingabe, "Nein")))
	{
		printf("Bitte geben Sie das Verzeichnis, sowie den Dateinamen der Zieldatei ein, \ndie das Ergebnis des Vergleichs speichert (zum Beispiel: C:\\Ergebnis.txt): ");
		gets(zielDatei);
		while((p_datei = fopen(zielDatei, "w")) == 0)
		{
			printf("Bitte geben Sie das Verzeichnis, sowie den Dateinamen der Zieldatei ein, \ndie das Ergebnis des Vergleichs speichert (zum Beispiel: C:\\Ergebnis.txt): ");
			gets(zielDatei);
			fclose(p_datei);
		}
	}
	else if(!(strcmp(eingabe, "Ja")))
	{
		strcpy(zielDatei, vorschlag);
	}
}

int dateiExtTxt (char *eingabeFileName)				//Funktion prüft ob ein Filename mit ".txt" endet
{
	char *datExt;
	datExt = strrchr(eingabeFileName, '.');
	if (!strcmp(datExt, ".txt"))
	{
		return (1);
	}
    return (0);
}

void attributeAuslesen()
{
	DIR *p_verzeichnis;				//Zeiger auf ein Verzeichnis (zum Öffnen und Schließen)
	struct dirent *dateiZeiger;		//Zeiger auf eine Datei (durchgehen aller Dateien in einem Verzeichnis)
	struct stat attribut;			//Variable zum Auslesen der Dateigröße und des Änderungsdatums
	const char *finden;				//Zeiger zur Bestimmung des Dateityps
	char datum[27];					//Zwischenspeicher für das Änderungsdatum der Datei
	char *datExt;
	
	/*Zeiger für die Liste 1 für das Verzeichnis 1*/
	datei *listeVerEins;
	datei *startVerEins = 0;
	datei *tmpVerEins = 0;
	
	/*Zeiger für die Liste 2 für das Verzeichnis 2*/
	datei *listeVerZwei;
	datei *startVerZwei = 0;
	datei *tmpVerZwei = 0;
	
	/*Dateien in beiden Verzeichnissen ermitteln und diese in einer Liste vom Typ Datei speichern*/
	/*beginnend mit Verzeichnis 1 --> speichern in Liste 1*/
	if((p_verzeichnis = opendir(verzeichnisEins)) != NULL)
	{
		
		chdir(verzeichnisEins);				//Working-Directory auf VerzeichnisEins ändern
		while((dateiZeiger = readdir(p_verzeichnis)) != NULL)
		{
			/*Speicherplatz für aktuelle Struktur anlegen, die Adresse steht nun in "listeVerEins"*/
			listeVerEins = malloc(sizeof(datei));
			
			/*Ermittlung, ob Element das erste in der Liste 1 ist*/
			if (!tmpVerEins) 	//Element ist das erste der Liste 1	
			{
				startVerEins = listeVerEins;
			}
			else				//Element ist nicht das erste der Liste 1
			{
				tmpVerEins->next = listeVerEins;   		
			}
					
			/*Dateinamen für aktuelle Datei auslesen und für aktuelles Element in Liste speichern*/
				
			strcpy(listeVerEins->dateiname, dateiZeiger->d_name);
			/*Dateigröße für aktuelle Datei auslesen und für aktuelles Element in Liste speichern*/
			stat(listeVerEins->dateiname, &attribut);
			listeVerEins->dateiGroesse = attribut.st_size;
			/*Änderungsdatum für die aktuelle Datei bestimmen und für aktuelles Element in die Liste speichern*/
			strcpy(listeVerEins->datum, (ctime(&attribut.st_mtime)));
			strcpy(datum, (ctime(&attribut.st_mtime)));
			listeVerEins->datum[0] = datum[8];
			listeVerEins->datum[1] = datum[9];
			listeVerEins->datum[2] = ' ';
			listeVerEins->datum[3] = datum[4];
			listeVerEins->datum[4] = datum[5];
			listeVerEins->datum[5] = datum[6];
			listeVerEins->datum[6] = ' ';
			listeVerEins->datum[7] = datum[20];
			listeVerEins->datum[8] = datum[21];
			listeVerEins->datum[9] = datum[22];
			listeVerEins->datum[10] = datum[23];
			listeVerEins->datum[11] = '\0';
			
			/*Dateityp für aktuelle Datei bestimmen und für aktuelles Element in Liste speichern*/
			datExt = strrchr(listeVerEins->dateiname, '.');	
			if (datExt != NULL)	
			{								
				strcpy(listeVerEins->typ, datExt);
			}
			
			/*vorsorglich aktuelles Element als letztes der Liste markieren*/
			listeVerEins->next = 0;
			/*der Zwischenspeicher erhält die Adresse des aktuellen Elements*/
			tmpVerEins = listeVerEins; 				
		}
		/*Verzeichnis 1 schließen*/
		closedir(p_verzeichnis);
		
		/*weitermachen mit Verzeichnis 2 --> speichern in Liste 2*/
		if((p_verzeichnis = opendir(verzeichnisZwei)) != NULL)
		{
			chdir(verzeichnisZwei);			//Working-Directory auf VerzeichnisZwei ändern
			while((dateiZeiger = readdir(p_verzeichnis)) != NULL)
			{
				/*Speicherplatz für aktuelle Struktur anlegen, die Adresse steht nun in "listeVerZwei"*/	
				listeVerZwei = malloc(sizeof(datei));
				
				/*Ermittlung, ob Element das erste in der Liste 2 ist*/
				if (!tmpVerZwei) //Element ist das erste der Liste 2
				{
					startVerZwei = listeVerZwei; 			
				}
				else //Element ist nicht das erste der Liste 2
				{
					tmpVerZwei->next = listeVerZwei;   		
				}
				
				/*Dateinamen für aktuelle Datei auslesen und für aktuelles Element in Liste speichern*/
				strcpy(listeVerZwei->dateiname, dateiZeiger->d_name);
				
				/*Dateigröße für aktuelle Datei auslesen und für aktuelles Element in Liste speichern*/
				stat(listeVerZwei->dateiname, &attribut);
				listeVerZwei->dateiGroesse = attribut.st_size;
				/*Änderungsdatum für die aktuelle Datei bestimmen und für aktuelles Element in die Liste speichern*/
				strcpy(datum, (ctime(&attribut.st_mtime)));
				listeVerZwei->datum[0] = datum[8];
				listeVerZwei->datum[1] = datum[9];
				listeVerZwei->datum[2] = ' ';
				listeVerZwei->datum[3] = datum[4];
				listeVerZwei->datum[4] = datum[5];
				listeVerZwei->datum[5] = datum[6];
				listeVerZwei->datum[6] = ' ';
				listeVerZwei->datum[7] = datum[20];
				listeVerZwei->datum[8] = datum[21];
				listeVerZwei->datum[9] = datum[22];
				listeVerZwei->datum[10] = datum[23];
				listeVerZwei->datum[11] = '\0';
			
				/*Dateityp für aktuelle Datei bestimmen und für aktuelles Element in Liste speichern*/
				datExt = strrchr(listeVerZwei->dateiname, '.');	
				if (datExt != NULL)
				{								
					strcpy(listeVerZwei->typ, datExt);
				}
				/*vorsorglich aktuelles Element als letztes der Liste markieren*/
				listeVerZwei->next = 0;
				/*der Zwischenspeicher erhält die Adresse des aktuellen Elements*/
				tmpVerZwei = listeVerZwei; 
			}
		}
		/*Verzeichnis 2 schließen*/
		closedir(p_verzeichnis);
	}	
	/*Vergleich innerhalb der Liste 1 (Verzeichnis 1) auf gleiche Elemente*/
	vergleich(duplikate, startVerEins, startVerEins);
	/*Vergleich innerhalb der Liste 2 (Verzeichnis 2) auf gleiche Elemente*/
	vergleich(duplikate, startVerZwei, startVerZwei);
	/*Vergleich beider Listen auf gleiche Elemente (ein Element aus Liste 1 mit allen aus Liste 2)*/
	vergleich(duplikate, startVerEins, startVerZwei);
}

void vergleich(struct paare *dup, datei *listeEins, datei *listeZwei)
{
	datei *startListeZwei = listeZwei;			//Kopie für Startadresse der zweiten Liste
	int i;										//Zählvariable für Duplikat-Array
		
	/*Prüfung auf dopplete Dateien*/	
	while (listeEins)
	{
		while (listeZwei)
		{
			//Ausschluss vom Vergleich der Dateien mit sich selbst
			if(listeEins != listeZwei)
			{
				//filtert . und .. Dateien heraus
				if((strcmp(listeEins->dateiname,".")) & (strcmp(listeZwei->dateiname,".")) & (strcmp(listeEins->dateiname,"..")) & (strcmp(listeZwei->dateiname,"..")))
				{
					//sucht nach Übereinstimmungen von Dateitypen und Dateigrößen und identifiziert somit Duplikate
					if(((strcmp(listeEins->typ, listeZwei->typ)) == 0) && (listeEins->dateiGroesse == listeZwei->dateiGroesse))
					{
						strcpy(dup[dup_index].nameDatEins, listeEins->dateiname);
						strcpy(dup[dup_index].nameDatZwei, listeZwei->dateiname);
						strcpy(dup[dup_index].datumDatEins, listeEins->datum);
						strcpy(dup[dup_index].datumDatZwei, listeZwei->datum);
						strcpy(dup[dup_index].typDatEins, listeEins->typ);
						strcpy(dup[dup_index].typDatZwei, listeZwei->typ);
						dup[dup_index].grDatEins = listeEins->dateiGroesse;
						dup[dup_index].grDatZwei = listeZwei->dateiGroesse;
						dup_index++;
						
						//filtert identische Zeilen von Duplikaten heraus (Kreuzbezug)
						for(i = 0; i < dup_index; i++)
						{
							if(!strcmp(listeZwei->dateiname, dup[i].nameDatEins))
							{
						 		dup_index--;
						 		i = dup_index;
							}
						}
					}
				}
			}

			/*nächstes Element der Liste 1 aufrufen*/
			listeZwei = listeZwei->next;
		}
		/*nächstes Element der Liste 1 aufrufen*/
		listeEins = listeEins->next;
		/*Zwischenspeicher der Liste 2 auf Startwert setzen*/
		listeZwei = startListeZwei;
	}
}

void zielDateiSchreiben(struct paare *dup)
{
	int i;
	FILE *file_zielDat;
	
	file_zielDat = fopen(zielDatei, "w");
	if(file_zielDat)
	{
		fprintf(file_zielDat, "Finddup - Doppelte Dateien finden\nMatrikelnummer: 2125287\n\n\nFolgende wahrscheinlich doppelte Dateien wurden gefunden:\n");
		fprintf(file_zielDat, "Verzeichnis 1: %s\n", verzeichnisEins);
		fprintf(file_zielDat, "Verzeichnis 2: %s\n\n", verzeichnisZwei);
		fprintf(file_zielDat, "Dateiname%10sDatum%8sTyp%4sGroesse%1s | ", "", "", "", "");
		fprintf(file_zielDat, "Dateiname%10sDatum%8sTyp%4sGroesse%1s\n", "", "", "", "");
		fprintf(file_zielDat, "------------------------------------------------+");
		fprintf(file_zielDat, "-------------------------------------------------\n");
		for (i = 0; i < dup_index; i++)
		{
			fprintf(file_zielDat, "%-19s%-13s%-7s%-9ld| ", dup[i].nameDatEins, dup[i].datumDatEins, dup[i].typDatEins, dup[i].grDatEins);
			fprintf(file_zielDat, "%-19s%-13s%-7s%-9ld\n", dup[i].nameDatZwei, dup[i].datumDatZwei, dup[i].typDatZwei, dup[i].grDatZwei);
		}		
		fprintf(file_zielDat, "------------------------------------------------+");
		fprintf(file_zielDat, "-------------------------------------------------\n");
		fprintf(file_zielDat, "Insgesamt wurden %u Uebereinstimmungen gefunden!\n\n", dup_index);
	}
	fclose(file_zielDat);
}

void ausgabe(struct paare *dup)				//Ausgabe der Ergebnisse des Vergleichs auf dem Bildschirm
{
	int i;
	system("cls");
	printf("Finddup - Doppelte Dateien finden\n");
	printf("Matrikelnummer: 2125287\n\n\n");
	
	printf("Folgende wahrscheinlich doppelte Dateien wurden gefunden:\n\n");
	printf("Verzeichnis 1: %s\n", verzeichnisEins);
	printf("Verzeichnis 2: %s\n\n", verzeichnisZwei);
	printf("Dateiname%10sDatum%8sTyp%4sGr%c%ce%3s | ", "", "", "", 148, 225, "");
	printf("Dateiname%10sDatum%8sTyp%4sGr%c%ce%3s\n", "", "", "", 148, 225, "");
	printf("------------------------------------------------+");
	printf("-------------------------------------------------\n");
	for (i = 0; i < dup_index; i++)
	{
		printf("%-19s%-13s%-7s%-9ld| ", dup[i].nameDatEins, dup[i].datumDatEins, dup[i].typDatEins, dup[i].grDatEins);
		printf("%-19s%-13s%-7s%-9ld\n", dup[i].nameDatZwei, dup[i].datumDatZwei, dup[i].typDatZwei, dup[i].grDatZwei);
	}		
	printf("------------------------------------------------+");
	printf("-------------------------------------------------\n");
	printf("Insgesamt wurden %u %cbereinstimmungen gefunden!\n\n", dup_index, 154);
}
