#include "trip.hpp"

TripStation::TripStation(int stationId, int seq, int arrival, int departure) {
    stationId_ = stationId;
    seq_ = seq;
    arrival_ = arrival;
    departure_ = departure;
}

bool UniqueSequenceStations::operator()(const TripStation &a,
                                        const TripStation &b) const {
    if (a.seq_ == b.seq_ && a.stationId_ == b.stationId_) return false;
    return a.seq_ < b.seq_;
}

void Trip::addStation(int seq, int stationId, int arrival, int departure) {
    stations_.insert({stationId, seq, arrival, departure});
}
