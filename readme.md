#What is this?

This program utilizes the Google Maps API to plot points on the map based on information pulled from a textfile.


#Information Format

|ID    | Temp (c)| Date and Time    | Latitude | Longitude  | Region ID (Depricated)       |
|------|---------|------------------|----------|------------|------------------------------|
| 1    |  8.50   | 2013 11 02 15 32 | 48.46988 | 236.68118  | 5                            |

##ID

The ID is used to retreive the name from the hardcoded internal dictionairy of station names

###Adding new station names

To add a new station name, simply give it an unused ID, and map that key to a name.

##Temperature

Temperature is used to determine the color of the pushpin.

##Temperature Key

t = temperature
A = average temperature

| Blue       | Green            | Yellow         | Red        |
|------------|------------------|----------------|------------|
| (t-A) < -1 | -1 <= (t-A) < 0  | 0 <= (t-A) < 1 | 1 <= (t-A) |

##Latitude and Longitude

Latitude and Longitude are used to determine where to plot the pushpin on the Google Maps canvas

##Region ID (Depricated)

No longer in use, the region ID served for region mapping and classification
