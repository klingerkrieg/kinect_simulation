// client_and_server.C
//	This is a VRPN example program that has a client and server for a
// tracker both within the same thread.
//	The basic idea is to instantiate both a vrpn_Tracker_NULL and
// a vrpn_Tracker_Remote using the same connection for each. Then, the
// local call handlers on the connection will send the information from
// the server to the client callbacks.

#include <stdio.h>                      // for fprintf, stderr, printf, etc

#include "vrpn_Configure.h"             // for VRPN_CALLBACK, etc
#include "vrpn_Connection.h"
#include "vrpn_Shared.h"                // for vrpn_SleepMsecs
#include "vrpn_Tracker.h"               // for vrpn_TRACKERCB, etc
#include "vrpn_Types.h"                 // for vrpn_float64
#include <vector>                // for vector

using namespace std;

const char	*TRACKER_NAME = "Tracker0";
int	CONNECTION_PORT = vrpn_DEFAULT_LISTEN_PORT_NO;	// Port for connection to listen on

vrpn_Tracker_Server	*ntkr;
vrpn_Tracker_Remote	*tkr;
vrpn_Connection		*connection;
unsigned tracker_stride = 1; // Every nth report will be printed

class t_user_callback {
public:
    char t_name[vrpn_MAX_TEXT_LEN];
    vector<unsigned> t_counts;
};



/*****************************************************************************
 *
   Callback handlers
 *
 *****************************************************************************/

void VRPN_CALLBACK
handle_tracker_pos_quat(void *userdata, const vrpn_TRACKERCB t)
{
    t_user_callback *t_data = static_cast<t_user_callback *>(userdata);
	
    // Make sure we have a count value for this sensor
    while (t_data->t_counts.size() <= static_cast<unsigned>(t.sensor)) {
        t_data->t_counts.push_back(0);
    }

	
    // See if we have gotten enough reports from this sensor that we should
    // print this one.  If so, print and reset the count.
    if (++t_data->t_counts[t.sensor] >= tracker_stride) {

        t_data->t_counts[t.sensor] = 0;
        printf("Tracker %s, sensor %d:\n     pos (%5.2f, %5.2f, %5.2f); "
               "quat (%5.2f, %5.2f, %5.2f, %5.2f)\n",
               t_data->t_name, t.sensor, t.pos[0], t.pos[1], t.pos[2],
               t.quat[0], t.quat[1], t.quat[2], t.quat[3]);
    }
}



struct Pos{
	float pos1;
	float pos2;
	float pos3;
};

struct Quat{
	float quat1;
	float quat2;
	float quat3;
	float quat4;
};




void readMockFile(int sensors[], float positions[][3], float quaterions[][4], int size){

	char str[512];
	FILE * file;
	file = fopen( "mock2.txt" , "r");

	int i = 0;
	
	if (file) {
		while ( fgets (str , 100 , file) != NULL ){
			//Caso seja comentario
			if (str[0] != 's'){
				continue;
			}

			//pula sensor
			char * part = strtok (str,"\t");
			part = strtok (NULL, " \t");
			int sensor = atoi(part);
			
			//pula pos
			part = strtok (NULL, " \t");
			part = strtok (NULL, " \t");
			double pos1 = atof(part);

			part = strtok (NULL, " \t");
			double pos2 = atof(part);

			part = strtok (NULL, " \t");
			double pos3 = atof(part);

			//pula quat
			part = strtok (NULL, " \t");
			part = strtok (NULL, " \t");
			double quat1 = atof(part);

			part = strtok (NULL, " \t");
			double quat2 = atof(part);

			part = strtok (NULL, " \t");
			double quat3 = atof(part);

			part = strtok (NULL, " \n");
			double quat4 = atof(part);


			//printf("sensor: [%d] pos:[%.2f][%.2f][%.2f] quat: [%.2f][%.2f][%.2f][%.2f]\n", sensor, pos1, pos2, pos3, quat1, quat2, quat3, quat4);
			/*t[i].sensor = sensor;
			t[i].pos[0] = pos1;
			t[i].pos[1] = pos2;
			t[i].pos[2] = pos3;
			t[i].quat[0] = quat1;
			t[i].quat[1] = quat2;
			t[i].quat[2] = quat3;
			t[i].quat[3] = quat4;*/
			sensors[i] = sensor;
			positions[i][0] = (float)pos1;
			positions[i][1] = (float)pos2;
			positions[i][2] = (float)pos2;
			quaterions[i][0] = (float)quat1;
			quaterions[i][1] = (float)quat2;
			quaterions[i][2] = (float)quat3;
			quaterions[i][3] = (float)quat4;


			
			i++;
			if (size == i){
				break;
			}
		}
		fclose(file);
	}

}


int main (int argc, char * argv []) {

	//esse numero nao pode ser maior do que a quantidade de linhas com dados no arquivo de mock
	const int lenMock = 31000;
	//vrpn_TRACKERCB tMocked[15000];
	int sensors[lenMock];
	float positions[lenMock][3];
	float quaternions[lenMock][4];


	readMockFile(sensors, positions, quaternions, lenMock);
	//return -1;

	printf("default port: %d \n", CONNECTION_PORT);
	if (argc != 1) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return -1;
	}

	// explicitly open the connection
	connection = vrpn_create_server_connection(CONNECTION_PORT);

	// Open the tracker server, using this connection, 2 sensors, update 60 times/sec
	ntkr = new vrpn_Tracker_Server(TRACKER_NAME, connection, 20);

	
	
	// Open the tracker remote using this connection
	
	tkr = new vrpn_Tracker_Remote (TRACKER_NAME, connection);
	t_user_callback *tc1 = new t_user_callback;
	strncpy(tc1->t_name, TRACKER_NAME, sizeof(tc1->t_name));

	fprintf(stderr, "Tracker's name is %s.\n", TRACKER_NAME);

	

	// Set up the tracker callback handlers
	//printf("Tracker update: '.' = pos, '/' = vel, '~' = acc\n");
	tkr->register_change_handler(tc1, handle_tracker_pos_quat);

	/* 
	 * main interactive loop
	 */
	int i = 0;
	while ( 1 ) {
		// Let the tracker server, client and connection do their things
		ntkr->mainloop();
		//ntkr->server_mainloop();
		
		//vrpn_uint32 sensor = tMocked[i].sensor;
		timeval t;
		t.tv_sec = 1;
		t.tv_usec = 1;
		vrpn_float64 position[3] = {positions[i][0], positions[i][1], positions[i][2]};
		vrpn_float64 quaternion[4] = {quaternions[i][0], quaternions[i][1], quaternions[i][2], quaternions[i][3]};
		

		//ntkr->report_pose(tMocked[i].sensor,t,tMocked[i].pos,tMocked[i].quat,vrpn_CONNECTION_LOW_LATENCY);
		ntkr->report_pose(sensors[i],t, position, quaternion,vrpn_CONNECTION_LOW_LATENCY);

		tkr->mainloop();
		connection->mainloop();

		// Sleep for 1ms so we don't eat the CPU
		vrpn_SleepMsecs(1);
		i++;

		if (i == lenMock){
			i = 0;
		}
	}

	return 0;

}   /* main */


