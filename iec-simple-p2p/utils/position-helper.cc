#include "position-helper.h"

using namespace ns3;

// Function to get the IP-Address as string from a node
void PositionHelper::setPosition(Ptr<Node> node, double x, double y) {
     Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
     if (mobility != NULL) {
          mobility->SetPosition(Vector(x, y, 0));
     }
}
