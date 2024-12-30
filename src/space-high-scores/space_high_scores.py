import zmq
import os
import sys

sys.path.append("src/proto")
from scores_pb2 import ScoresMessage  # type: ignore


COMMS_H_FILE_PATH = "include/comms.h"
SCORES_UPDATE_TOPIC = 2  # From PUBSUB_TOPICS enum in include/comms.h


def extract_server_info() -> str:
    """Extracts server info from comms.h"""

    definitions = {"PROTOCOL": None, "SERVER_IP": None, "PORT_PUBSUB": None}

    try:
        with open(COMMS_H_FILE_PATH, "r") as file:
            for line in file:
                line = line.strip()

                # Search for example for:
                # #define PROTOCOL "tcp"
                if line.startswith("#define"):
                    parts = line.split()
                    if len(parts) == 3:
                        key, value = parts[1], parts[2]
                        if key in definitions:
                            definitions[key] = value.replace('"', "")  # Remove ""
    except Exception as e:
        print(f"An error occurred: {e}")

    return f"{definitions['PROTOCOL']}://{definitions['SERVER_IP']}:{definitions['PORT_PUBSUB']}"


def display_scoreboard(scores: list):
    """Displays the scoreboard"""

    # Convert user id to the respective letter
    id_to_symbol = lambda id: chr(ord("A") + id)

    scores_with_symbols = [
        {"symbol": id_to_symbol(index), "score": score}
        for index, score in enumerate(scores)
    ]

    os.system("cls" if os.name == "nt" else "clear")

    print("====== Scoreboard ======")
    sorted_scores_with_symbols = sorted(
        scores_with_symbols, key=lambda x: x["score"], reverse=True
    )
    for info in sorted_scores_with_symbols:
        if info["score"] != -1:
            string = f"{info['symbol']} - {info['score']:3}"
            print(f"{string:^24}")
    print("========================")


def main():

    zmp_server_path = extract_server_info()

    context = zmq.Context()
    socket = context.socket(zmq.SUB)
    socket.connect(zmp_server_path)
    socket.setsockopt(
        zmq.SUBSCRIBE,
        SCORES_UPDATE_TOPIC.to_bytes(
            4, byteorder="little"
        ),  # Match the way C encodes integers into bytes
    )

    print("Waiting for score updates...")
    try:
        while True:
            _ = socket.recv()  # topic isn't needed
            _ = socket.recv()  # msg type isn't needed
            proto_message = socket.recv()

            scores_message = ScoresMessage()
            scores_message.ParseFromString(proto_message)

            display_scoreboard(scores_message.scores)
    except KeyboardInterrupt:
        socket.close()
        context.term()


if __name__ == "__main__":
    main()
