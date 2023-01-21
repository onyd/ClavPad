import json
import os

if __name__ == "__main__":
    activity = [(x.keys(), x.values()) for x in json.load(open("activity.json", mode="r"))]

    current_time = 0
    mouse_moves = []
    homings = []
    
    last_click_time = None
    last_kb_time = None
    for entry in activity:
        mode, duration = list(entry[0])[0], list(entry[1])[0]
        if mode == "mouse_move":
            mouse_moves.append(duration)
            if last_kb_time is not None:
                homings.append(current_time - last_kb_time)

        elif mode == "mouse_click":
            last_click_time = current_time 

        elif mode == "keyboard":
            last_kb_time = current_time
            if last_click_time is not None:
                homings.append(current_time - last_click_time)

        current_time += duration

    print("total time (ms): ", current_time)
    print("mouse moves (ms): ", mouse_moves)
    print("homings (ms): ", homings)