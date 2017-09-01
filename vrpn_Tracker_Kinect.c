
void vrpn_Tracker_Kinect::mainloop() {
    struct timeval current_time;
    char msgbuf[1000];
    vrpn_int32 i, len;

    // Call the generic server mainloop routine, since this is a server
    server_mainloop();

    // See if its time to generate a new report
    vrpn_gettimeofday(&current_time, NULL);
    if (vrpn_TimevalDuration(current_time, timestamp) >=
        1000000.0 / update_rate) {

        // Update the time
        timestamp.tv_sec = current_time.tv_sec;
        timestamp.tv_usec = current_time.tv_usec;

        // Send messages for all sensors if we have a connection
        if (d_redundancy) {
            for (i = 0; i < num_sensors; i++) {
                d_sensor = i;

                // Pack position report
                len = encode_to(msgbuf);
                if (d_redundancy->pack_message(len, timestamp, position_m_id,
                                               d_sender_id, msgbuf,
                                               vrpn_CONNECTION_LOW_LATENCY)) {
                    fprintf(stderr,
                            "NULL tracker: can't write message: tossing\n");
                }

            }
        }
        else if (d_connection) {
            for (i = 0; i < num_sensors; i++) {
                d_sensor = i;

                // Pack position report
                len = encode_to(msgbuf);
                if (d_connection->pack_message(len, timestamp, position_m_id,
                                               d_sender_id, msgbuf,
                                               vrpn_CONNECTION_LOW_LATENCY)) {
                    fprintf(stderr,
                            "NULL tracker: can't write message: tossing\n");
                }

            }
        }
    }
}

void vrpn_Tracker_Kinect::setRedundantTransmission(vrpn_RedundantTransmission *t) {
    d_redundancy = t;
}
