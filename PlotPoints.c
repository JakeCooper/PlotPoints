/*
 *  Author:      Jake Cooper
 *  Date:        November 14th, 2013
 *  File name:   PlotPoints.c
 *  Description: Reads data from a text file, and outputs an HTML file with pinouts that have qualities listed in the text file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ========================================================================= */
/*                              Type Definitions                             */
/*          Do not alter the provided definitions (they will be used         */
/*            for marking). You may add new definitions as needed.           */
/* ========================================================================= */

/* The name of the input file */
#define INPUT_FILENAME "Plotinput.txt"

/* The name of the output file */
#define OUTPUT_FILENAME "Plotoutput.html"

/* Latitude and Longitude coordinates of the ECS Building */
#define ECS_LATITUDE 48.46104
#define ECS_LONGITUDE -123.31153

/* Maximum number of stations allowed. Currently there are 201 VWSN stations. */
#define MAX_STATIONS 201

/* A struct defining a point on the Earth's surface */
typedef struct{
        float latitude;
        float longitude;
} GeographicPoint;

/* A struct describing a station and its current temperature */
typedef struct{
        /* The numerical ID of the station */
        int stationID;
        /* The geographic location of the station */
        GeographicPoint location;
        /* The temperature at the station */
        float temperature;
        /* The time of the temperature observation */
        int year, month, day, hour, minute;
} StationData;

/* Available marker colours */
#define MARKER_RED 0
#define MARKER_BLUE 1
#define MARKER_GREEN 2
#define MARKER_YELLOW 3
#define MARKER_PURPLE 4

/* A struct describing a marker on the map. */
typedef struct{
        /* The geographic location of the marker */
        GeographicPoint location;
        /* markerName should be a short name for the marker (e.g. 'UVic Science Building') */
        /* markerName must be a null-terminated C string */
        char markerName[100];
        /* markerText should contain the text to be displayed when the marker is clicked. */
        /* Note that this may contain HTML tags (although using them is not required) */
        /* markerText must be a null-terminated C string */
        char markerText[1000];
        /* The colour of the marker. This should be set to one of the constants defined above. */
        int type;
} MapMarker;

/* ========================================================================= */
/*                       Library Function  Declarations                      */
/*            These functions are defined at the end of the file.            */
/*      (Do not modify these declarations or the corresponding functions)    */
/* ========================================================================= */


/* writePrologue
   This function writes the initial part of the HTML file (which sets up the Google
   maps interface). This must be called before any points are written to the file.
*/
void writePrologue(FILE *f);

/* writeEpilogue
   This function writes the final part of the HTML file. This must be called
   after all points have been written, but before the file is closed.
   (The caller is responsible for closing the file afterwards).
*/
void writeEpilogue(FILE *f);

/* writePoint
   This function adds a Google Maps marker to the output file for the MapMarker
   provided.
*/
void writePoint(FILE *f, MapMarker *marker);

/* getStationName
   This function takes a numerical VWSN station ID, and returns a C string
   containing the station name, or "Unknown" if no station with that name is found.
*/
char* getStationName(int stationID);

/* surfaceDistance
   Given two geographic points, compute the distance between the two points in kilometers.
*/
float surfaceDistance(GeographicPoint* point1, GeographicPoint* point2);

/* ========================================================================= */
/*                              Main Program                                 */
/*                          												 */
/*            - Blue    if        (t-A) < -1                                 */
/*            - Green   if  -1 <= (t-A) < 0                                  */
/*            - Yellow  if   0 <= (t-A) < 1                                  */
/*            - Red     if   1 <= (t-A)                                      */
/*                                                                           */
/* ========================================================================= */

int main(){

        /* Create an array of StationData structs to store all station data points */
        StationData stationInfo[MAX_STATIONS];

        /* Open the input file */
        FILE *inFp;
        inFp = fopen(INPUT_FILENAME, "r");
        if (inFp == NULL){
                printf("File %s cannot be opened\n", INPUT_FILENAME);
                return EXIT_FAILURE;
        }

        /*initialize array at 0*/
        int i;
        for(i=0; i < MAX_STATIONS; i++){
                stationInfo[i].stationID = 0;
                stationInfo[i].temperature = 0;
                stationInfo[i].year = 0;
                stationInfo[i].month = 0;
                stationInfo[i].day = 0;
                stationInfo[i].hour = 0;
                stationInfo[i].minute = 0;
                stationInfo[i].location.latitude = 0;
                stationInfo[i].location.longitude = 0;

        }

        /* Read each line of the input file into a StationData struct in the data array */
        int j;
        float regionID;
        int t = 0;
        char string[100];
        float totalTemp = 0;
        for(j = 0; j < MAX_STATIONS; j++){

                if (fgets(string, sizeof(string), inFp) == 0){
                        break; /*If there is nothing in the line, break the loop*/
                }

                sscanf(string,"%d %f %d %d %d %d %d %f %f %f", &stationInfo[j].stationID, &stationInfo[j].temperature, &stationInfo[j].year, &stationInfo[j].month, &stationInfo[j].day, &stationInfo[j].hour, &stationInfo[j].minute, &stationInfo[j].location.latitude, &stationInfo[j].location.longitude, &regionID);

                if(stationInfo[j].temperature != 0){
                        t++; /*Number of stations with non-zero temperatures, and thus number of stations*/
                }

                totalTemp += stationInfo[j].temperature; /*total temperatures of stations*/
        }

        /* Close the input file */
        fclose(inFp);

        /* Compute the average temperature of all stations */
        float avgTemp = (totalTemp/t);

        /* Open the output file */
        FILE *outFp;
                outFp = fopen(OUTPUT_FILENAME, "w");
                if (outFp == NULL){
                        printf("File %s cannot be opened\n", OUTPUT_FILENAME);
                        return EXIT_FAILURE;
                }

        /* Write the Prologue to the output file */
        writePrologue(outFp);

        /* For each station, create a marker at the correct position containing the
           station's temperature. Give the marker a name (using the getStationName
           function) and description text containing the temperature.
           Use the marker colour scheme described above. */
        MapMarker mapInfo[MAX_STATIONS];
        int k;
        for(k=0; k < t; k++){
                /*writes the latitude and logitude to location in struct(mapInfo)*/
                mapInfo[k].location.latitude = stationInfo[k].location.latitude;
                mapInfo[k].location.longitude = stationInfo[k].location.longitude;
                char dateStamp[1000];

                /*writes the stations name to markerName in struct(mapInfo)*/
                char* stationName = getStationName(stationInfo[k].stationID);
                strcpy(mapInfo[k].markerName, stationName);

                /*writes name (In bold), temperature and time to markerText in struct(mapInfo)*/
                char textInMarker[50];
                char boldName[100];
                sprintf(dateStamp, "(%d:%d %d/%d/%d)", stationInfo[k].hour, stationInfo[k].minute, stationInfo[k].month, stationInfo[k].day, stationInfo[k].year);
                sprintf(boldName, "<b>%s</b>", mapInfo[k].markerName );
                sprintf(textInMarker, "%s: %1.2f degrees %s", boldName, stationInfo[k].temperature, dateStamp);
                strcpy(mapInfo[k].markerText, textInMarker);

                /*determine the colour of the pins*/
                if((stationInfo[k].temperature - avgTemp) < -1){
                        mapInfo[k].type = MARKER_BLUE;
                }
                if((-1 <= (stationInfo[k].temperature - avgTemp)) && ((stationInfo[k].temperature - avgTemp) < 0)){
                        mapInfo[k].type = MARKER_GREEN;
                }
                if((0 <= (stationInfo[k].temperature - avgTemp)) && ((stationInfo[k].temperature - avgTemp) < 1)){
                        mapInfo[k].type = MARKER_YELLOW;
                }
                if(1 <= (stationInfo[k].temperature - avgTemp)){
                        mapInfo[k].type = MARKER_RED;
                }
                writePoint(outFp, &mapInfo[k]);
        }

        /* Compute the average temperature at all stations within 2km from the point
           (ECS_LATITUDE,ECS_LONGITUDE) to approximate the temperature at the ECS
           building. Create a purple marker named "ECS Building" containing the
           approximate temperature in its description.
           The surfaceDistance function should be used to compute the distance between
           pairs of (latitude, longitude) points. */

        float tempWithin2km = 0;
        float ecsTemp = 0;
        int numValidPoints = 0;
        MapMarker ECS;
        ECS.location.latitude = ECS_LATITUDE;
        ECS.location.longitude = ECS_LONGITUDE;
        int x;
        float distanceBetween;

        /*Calculating Avg temp if distance <= 2km from ECS*/
        for(x=0; x < t; x++){
        distanceBetween = surfaceDistance(&ECS.location, &mapInfo[x].location);
                if (distanceBetween <= 2) {
                        tempWithin2km += stationInfo[x].temperature;
                        numValidPoints++;
                }
        }
        ecsTemp = (tempWithin2km / numValidPoints);

        /*Giving the ECS struct it's required qualities*/

        ECS.location.latitude = ECS_LATITUDE;
        ECS.location.longitude = ECS_LONGITUDE;
        char ecsBody[100];
        sprintf(ecsBody, "<b>ECS Building</b>: %1.2f degrees", ecsTemp);
        strcpy(ECS.markerText, ecsBody);
        ECS.type = MARKER_PURPLE;
        strcpy(ECS.markerName, "ECS Building");
        writePoint(outFp,&ECS);

        /* Write the epilogue to the output file */
        writeEpilogue(outFp);
        /* Close the output file */
        fclose(outFp);
        return EXIT_SUCCESS;
}


/* ========================================================================= */
/*                           Library Functions                               */
/*        These are declared above, and will be useful to generate           */
/*            the output file. Do not modify these functions.                */
/* ========================================================================= */

/* writePrologue
   This function writes the initial part of the HTML file (which sets up the Google
   maps interface). This must be called before any points are written to the file.
*/
void writePrologue(FILE *f){
        if (!f){
                printf("writePrologue error: output file == NULL\n");
                exit(1);
        }
        fputs("<!DOCTYPE html>\n",f);
        fputs("<html>\n",f);
        fputs("<head>\n",f);
        fputs("<meta name=\"viewport\" content=\"initial-scale=1.0, user-scalable=no\" />\n",f);
        fputs("<style type=\"text/css\">\n",f);
        fputs("  html { height: 100% }\n",f);
        fputs("  body { height: 100%; margin: 0; padding: 0 }\n",f);
        fputs("  #map_canvas { height: 100% }\n",f);
        fputs("</style>\n",f);
        fputs("<script type=\"text/javascript\"\n",f);
        fputs("    src=\"http://maps.googleapis.com/maps/api/js?sensor=true\">\n",f);
        fputs("</script>\n",f);
        fputs("<script type=\"text/javascript\">\n",f);
        fputs("  function initialize() {\n",f);
        fputs("    var latlng = new google.maps.LatLng(48.447,236.643);\n",f);
        fputs("    var myOptions = {\n",f);
        fputs("      zoom: 13,\n",f);
        fputs("      center: latlng,\n",f);
        fputs("      mapTypeId: google.maps.MapTypeId.ROADMAP\n",f);
        fputs("    };\n",f);
        fputs("    var map = new google.maps.Map(document.getElementById(\"map_canvas\"),\n",f);
        fputs("             myOptions);\n",f);
} /* writePrologue */

/* writeEpilogue
   This function writes the final part of the HTML file. This must be called
   after all points have been written, but before the file is closed.
   (The caller is responsible for closing the file afterwards).
*/
void writeEpilogue(FILE *f){
        if (!f){
                printf("writeEpilogue error: output file == NULL\n");
                exit(1);
        }
        fputs("  }\n",f);
        fputs("</script>\n",f);
        fputs("</head>\n",f);
        fputs("<body onload=\"initialize()\">\n",f);
        fputs("  <div id=\"map_canvas\" style=\"width:100%; height:100%\"></div>\n",f);
        fputs("</body>\n",f);
        fputs("</html>\n",f);
} /* writeEpilogue */

/* Utility function to change ' to \' in strings */
void escapeQuotes(char *dest, char *src){
        while (*src){
                if (*src == '\'')
                        *dest++ = '\\';
                *dest++ = *src++;
        }
        *dest = '\0';
} /* escapeQuotes */

/* Utility function to implement the strnlen library function,
   which may not be available on some platforms
*/
int strnlen2(char* str, int maxSize){
        int i;
        for (i = 0; i < maxSize; i++)
                if (str[i] == '\0')
                        break;
        return i;
} /* strnlen2 */

#define NUM_MARKER_TYPES 5
char *MarkerPaths[] = {
        [MARKER_RED] = "http://maps.google.com/mapfiles/ms/icons/red-dot.png",
        [MARKER_GREEN] = "http://maps.google.com/mapfiles/ms/icons/green-dot.png",
        [MARKER_BLUE] = "http://maps.google.com/mapfiles/ms/icons/blue-dot.png",
        [MARKER_PURPLE] = "http://maps.google.com/mapfiles/ms/icons/purple-dot.png",
    [MARKER_YELLOW] = "http://maps.google.com/mapfiles/ms/icons/yellow-dot.png"
};

/* writePoint
   This function adds a Google Maps marker to the output file for the MapMarker
   provided.
*/
void writePoint(FILE *f, MapMarker *marker){
        static int numMarkers = 0;
        char escapedName[2*sizeof(marker->markerName)],escapedText[2*sizeof(marker->markerText)];
        int markerNum = numMarkers++;
        if (!f){
                printf("writePoint error: output file == NULL\n");
                exit(1);
        }
        /* Try to make sure that the input data is properly set */
        if (!marker){
                printf("writePoint error: marker == NULL\n");
                exit(1);
        }
        if (strnlen2(marker->markerName,sizeof(marker->markerName)) == sizeof(marker->markerName)){
                printf("writePoint error: marker->markerName is not null-terminated\n");
                exit(1);
        }
        if (strnlen2(marker->markerText,sizeof(marker->markerText)) == sizeof(marker->markerText)){
                printf("writePoint error: marker->markerText is not null-terminated\n");
                exit(1);
        }
        if (marker->type < 0 || marker->type >= NUM_MARKER_TYPES || !MarkerPaths[marker->type]){
                printf("writePoint error: invalid marker type\n");
                exit(1);
        }
        escapeQuotes(escapedName,marker->markerName);
        escapeQuotes(escapedText,marker->markerText);
        fprintf(f,"\tvar iw%d = new google.maps.InfoWindow({content: '%s'});\n",markerNum,escapedText);
        fprintf(f,"\tvar marker%d = new google.maps.Marker({position: new google.maps.LatLng(%f,%f), map: map, title: '%s', icon: '%s'});\n",
                                markerNum,marker->location.latitude,marker->location.longitude,escapedName,MarkerPaths[marker->type]);
        fprintf(f,"\tgoogle.maps.event.addListener(marker%d, 'click', function(){ iw%d.open(map,marker%d); } );\n",
                                markerNum, markerNum, markerNum);
} /* writePoint */

/* A large array of every VWSN station name, for use by the getStationName function */
/* Notice that the C99 sparse array initializer syntax is used here, since some entries */
/* of the array have no value (due to some stations being retired) */
char *AllStationNames[MAX_STATIONS] = {
[1] = "Ian Stewart Complex/Mt. Douglas High School",
[3] = "Strawberry Vale Elementary School",
[4] = "Oaklands Elementary School",
[5] = "Cedar Hill Middle School",
[6] = "Marigold Elementary School/Spectrum High School",
[7] = "Campus View Elementary",
[8] = "Victoria High School",
[9] = "Frank Hobbs Elementary School",
[10] = "MacAulay Elementary School",
[11] = "James Bay Elementary School",
[12] = "Victoria West Elementary School",
[13] = "Shoreline Middle School",
[14] = "Willows Elementary School",
[15] = "Sir James Douglas Elementary School",
[16] = "Tillicum Elementary School",
[17] = "Eagle View Elementary School",
[18] = "Torquay Elementary School",
[19] = "Monterey Middle School",
[20] = "Lake Hill Elementary School",
[21] = "Rogers Elementary School",
[22] = "Cloverdale Elementary School",
[24] = "Hillcrest Elementary School",
[25] = "Lansdowne Middle School",
[26] = "Doncaster Elementary School",
[27] = "Glanford Middle School",
[28] = "Sundance Elementary School",
[29] = "George Jay Elementary School",
[30] = "Northridge Elementary School",
[31] = "Sangster Elementary School",
[32] = "Colwood Elementary School",
[33] = "Reynolds High School",
[34] = "Crystal View Elementary School",
[35] = "David Cameron Elementary School",
[36] = "Hans Helgesen Elementary School",
[37] = "John Muir Elementary School",
[39] = "Lakewood Elementary School",
[40] = "Ruth King Elementary School",
[41] = "CTV Victoria",
[42] = "Butchart Gardens",
[46] = "CTV Nanaimo",
[50] = "Ocean Trails Resort",
[55] = "Savory Elementary School",
[56] = "Willway Elementary School",
[57] = "Wishart Elementary School",
[58] = "Dunsmuir Middle School",
[59] = "Journey Middle School/Poirier Elementary School",
[60] = "Esquimalt High School",
[61] = "Cordova Bay Elementary School",
[62] = "Deep Cove Elementary School",
[63] = "Keating Elementary School",
[64] = "Lochside Elementary School",
[66] = "Prospect Lake Elementary School",
[67] = "Sidney Elementary School",
[68] = "Bayside Middle School",
[70] = "Parkland Secondary School",
[71] = "Cal Revelle Nature Sanctuary",
[72] = "Race Rocks Ecological Reserve",
[73] = "Craigflower Elementary School",
[75] = "Central Middle School",
[76] = "Lambrick Park High School",
[77] = "McKenzie Elementary School",
[78] = "SJ Willis Alternative School",
[79] = "Arbutus Middle School",
[80] = "Gordon Head Middle School",
[81] = "Braefoot Elementary School",
[82] = "Colquitz Middle School",
[83] = "Winchelsea Elementary School",
[84] = "Qualicum Beach Middle School",
[85] = "Palsson Elementary School",
[86] = "Randerson Ridge Elementary School",
[88] = "PASS/Woodwinds Alternate School",
[89] = "Springwood Middle School",
[90] = "View Royal Elementary School",
[91] = "French Creek Community School",
[92] = "False Bay School",
[93] = "Shawnigan Lake Museum",
[94] = "Pender Islands Elementary and Secondary School",
[95] = "Arrowview Elementary School",
[96] = "Bowser Elementary School",
[97] = "Qualicum Beach Elementary School",
[98] = "Margaret Jenkins Elementary School",
[99] = "East Highlands District Firehall",
[100] = "District of Highlands Office",
[101] = "West Highlands District Firehall",
[103] = "Frances Kelsey Secondary School",
[104] = "Happy Valley Elementary School",
[105] = "Port Renfrew Elementary School",
[106] = "Edward Milne Community School",
[107] = "Millstream Elementary School",
[108] = "Alberni Weather",
[109] = "Brentwood Elementary School",
[110] = "Nanoose Bay Elementary School",
[111] = "Parksville Elementary School",
[112] = "Saturna Elementary School",
[113] = "Mayne Island Elementary &amp; Junior Secondary School",
[114] = "Galiano Island Community School",
[115] = "L'Ecole Victor Brodeur",
[117] = "Salt Spring Elementary School &amp;Saltspring Middle School",
[119] = "Fernwood Elementary School",
[120] = "Fulford Elementary School",
[121] = "Gulf Islands Secondary School",
[122] = "Phoenix Elementary School",
[123] = "Vancouver Island University",
[124] = "Seaview Elementary School",
[125] = "St. Patrick's Elementary School",
[126] = "Quamichan Middle School",
[127] = "Cowichan Valley Open Learning Cooperative",
[128] = "John Stubbs Memorial School",
[129] = "G.R. Paine Horticultural Training Centre",
[131] = "Glenlyon Norfolk Junior School",
[132] = "Shawnigan Lake",
[133] = "Discovery Elementary School",
[134] = "Swan Lake Nature House",
[136] = "Pleasant Valley Elementary School",
[137] = "McGirr Elementary School",
[138] = "Bayview Elementary School",
[139] = "L'Ecole Hammond Bay Elementary",
[140] = "Uplands Park Elementary",
[141] = "Mountain View Elementary",
[142] = "View Royal Fire Department",
[143] = "UVic Science Building",
[144] = "Elizabeth Buckley School - Cridge Centre",
[145] = "Chilliwack Education Centre",
[159] = "Camosun College Lansdowne",
[160] = "Shawnigan Lake School",
[161] = "Bamfield Marine Sciences Centre",
[162] = "St. Michaels University School Senior Campus",
[163] = "UVic Social Sciences and Mathematics Building",
[165] = "Alberni Elementary School",
[166] = "Maquinna Elementary School",
[167] = "Wikaninnish Community School",
[168] = "Ucluelet High School",
[169] = "Lighthouse Christian Academy",
[174] = "Kelset Elementary School",
[176] = "St. Michaels University School Junior Campus",
[177] = "Ladysmith Secondary School",
[179] = "Ray Watkins Elementary",
[180] = "Captain Meares Elementary Secondary School",
[181] = "West-Mont Montessori School",
[182] = "Pacific Biological Station, DFO-MPO",
[183] = "Brentwood College",
[184] = "NEPTUNE Port Alberni",
[185] = "Mt. Washington Alpine Resort-Nordic",
[186] = "Mt. Washington Alpine Resort-Alpine",
[187] = "Portage Inlet",
[188] = "North Saanich Middle School",
[189] = "Airport Elementary School",
[190] = "Courtenay Elementary School",
[191] = "Cumberland Junior Secondary School",
[192] = "Denman Island Community School",
[193] = "Hornby Island Community School",
[194] = "Miracle Beach Elementary",
[195] = "North Island Distance Education School",
[196] = "Valley View Elementary School",
[197] = "RASC Victoria Centre",
[199] = "Trial Island Lightstation",
[200] = "Longacre",
};

/* getStationName
   This function takes a numerical VWSN station ID, and returns a C string
   containing the station name, or "Unknown" if no station with that name is found.
*/
char* getStationName(int stationID){
        if (stationID < 0 || stationID >= MAX_STATIONS || !AllStationNames[stationID])
                return "Unknown";
        return AllStationNames[stationID];
} /* getStationName */

/* surfaceDistance
   Given two geographic points, compute the distance between the two points in kilometers.

   The formula used in this function is called the "Haversine formula". More information
   can be found at its Wikipedia page: http://en.wikipedia.org/wiki/Haversine_formula.
   Note that the distance computed here is approximate, since the Earth is not a perfect
   sphere, but the formula is sufficiently accurate for this assignment.
*/
float surfaceDistance(GeographicPoint* point1, GeographicPoint* point2){
        if (!point1){
                printf("surfaceDistance error: point1 is invalid\n");
                exit(1);
        }
        if (!point2){
                printf("surfaceDistance error: point2 is invalid\n");
                exit(1);
        }
        const float RADIUS_OF_EARTH = 6371;
        float lat1 = point1->latitude*M_PI/180.0;
        float lon1 = point1->longitude*M_PI/180.0;
        float lat2 = point2->latitude*M_PI/180.0;
        float lon2 = point2->longitude*M_PI/180.0;
        float lat_diff = fabs(lat1-lat2);
        float lon_diff = fabs(lon1-lon2);
        float latsin = sin(lat_diff/2);
        float lonsin = sin(lon_diff/2);
        float angle_diff = 2*asin(sqrt(latsin*latsin + cos(lat1)*cos(lat2)*lonsin*lonsin));
        return RADIUS_OF_EARTH*angle_diff;
} /* surfaceDistance */
