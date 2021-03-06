#
/*
 *    Copyright (C) 2010, 2011, 2012
 *    Jan van Katwijk (J.vanKatwijk@gmail.com)
 *    Lazy Chair Programming
 *
 *    This file is part of the SDR-J.
 *    Many of the ideas as implemented in SDR-J are derived from
 *    other work, made available through the GNU general Public License. 
 *    All copyrights of the original authors are recognized.
 *
 *    SDR-J is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    SDR-J is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with SDR-J; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	Main program
 */
#include	<QApplication>
#include	<QSettings>
#include	<QDir>
#include	<unistd.h>
#include	"dab-constants.h"
#include	"gui.h"

void	fullPathfor (const char *v, char *out) {
int16_t	i;
QString	homeDir;

	if (v == NULL) {
	   sprintf (out, "%s", "/tmp/xxx");
	   return;	// should not happen
	}

	if (v [0] == '/') {		// full path specified
	   sprintf (out, "%s", v);
	   return;
	}

	homeDir = QDir::homePath ();
	homeDir. append ("/");
	homeDir. append (v);
	homeDir	= QDir::toNativeSeparators (homeDir);
	sprintf (out, "%s", homeDir. toLatin1 (). data ());
	fprintf (stderr, "ini file = %s\n", out);

	for (i = 0; out [i] != 0; i ++);
	if (out [i - 4] != '.' ||
	    out [i - 3] != 'i' ||
	    out [i - 2] != 'n' ||
	    out [i - 1] != 'i') {
	    out [i] = '.';
	    out [i + 1] = 'i';
	    out [i + 2] = 'n';
	    out [i + 3] = 'i';
	    out [i + 4] = 0;
	}
}

bool	fileExists (char *v) {
FILE *f;

	f = fopen (v, "r");
	if (f == NULL)
	   return false;
	fclose (f);
	return true;
}

#define	DEFAULT_INI	".dab-rpi.ini"

int	main (int argc, char **argv) {
/*
 *	The default values
 */
char		*defaultInit		= (char *)alloca (512 * sizeof (char));
RadioInterface	*MyRadioInterface;
int32_t		opt;
uint8_t		syncMethod	= 2;
QSettings	*dabSettings;		// ini file
uint8_t		dabMode		= 127;	// illegal value
QString		dabDevice	= QString ("");
QString		dabBand		= QString ("");
	fullPathfor (DEFAULT_INI, defaultInit);

	while ((opt = getopt (argc, argv, "i:D:S:M:B:")) != -1) {
	   switch (opt) {
	      case 'i':
	         fullPathfor (optarg, defaultInit);
	         break;

	      case 'S':
	         syncMethod	= atoi (optarg);
	         break;
#ifdef GUI_3
	      case 'D':
	         dabDevice = optarg;
	         break;

	      case 'M':
	         dabMode	= atoi (optarg);
	         if (!(dabMode == 1) || (dabMode == 2) || (dabMode == 4))
	            dabMode = 1; 
	         break;

	      case 'B':
	         dabBand 	= optarg;
	         break;
#endif
	      default:
	         break;
	   }
	}
	dabSettings =  new QSettings (defaultInit, QSettings::IniFormat);

#ifdef	GUI_3
//	Since we do not have the possibility in GUI_3 to select
//	Mode, Band or Device, we create the possibility for
//	passing appropriate parameters to the command
//	Selections - if any - will be default for the next session

	if (dabMode == 127)
	   dabMode = dabSettings -> value ("dabMode", 1). toInt ();
	if (dabDevice == QString (""))
	   dabDevice = dabSettings -> value ("dabDevice", "dabstick"). toString ();
	if (dabBand == QString (""))
	   dabBand = dabSettings -> value ("band", "BAND III"). toString ();
#endif 
/*
 *	Before we connect control to the gui, we have to
 *	instantiate
 */
	QApplication a (argc, argv);
//	save the values for the new defaults
#ifdef	GUI_3
	dabSettings -> setValue ("dabMode", dabMode);
	dabSettings -> setValue ("dabDevice",  dabDevice);
	dabSettings -> setValue ("band",    dabBand);
	QQmlApplicationEngine engine(QUrl("qrc:/QML/main.qml"));
	MyRadioInterface = new RadioInterface (dabSettings, &engine,
	                                       dabDevice, dabMode, dabBand);
#else
#if QT_VERSION >= 0x050600
	QGuiApplication::setAttribute (Qt::AA_EnableHighDpiScaling);
#endif
	MyRadioInterface = new RadioInterface (dabSettings, syncMethod);
	MyRadioInterface -> show ();
#endif

	a. exec ();
/*
 *	done:
 */
	fflush (stdout);
	fflush (stderr);
	qDebug ("It is done\n");
	MyRadioInterface	-> ~RadioInterface ();
	dabSettings		-> ~QSettings ();
	exit (1);
}

