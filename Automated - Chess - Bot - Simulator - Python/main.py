# main.py
import multiprocessing
from chess_ui import ChessUI
from bots import RandomBot, PacifistBot, HumanPlayer, SmartBot
from cpp_bot import CppEngineBot
from secure_bot import SecureBotWrapper

if __name__ == "__main__":
    multiprocessing.freeze_support()
    
    print("--- CHESS TOURNAMENT MODE ---")
    print("1. C++ CheckMe vs Python SmartBot")
    print("2. Human vs C++ CheckMe")
    print("3. Human vs Python SmartBot")
    print("4. Bot vs Bot (Pacifist vs SmartBot)")
    print("5. Human vs Human")
    
    choice = input("Select Mode (1-5): ").strip()

    if choice == "1":
        player1 = SecureBotWrapper(CppEngineBot, "CheckMe (C++)")
        player2 = SecureBotWrapper(SmartBot, "SmartBot (Python)")

    elif choice == "2":
        player1 = HumanPlayer("You")
        player2 = SecureBotWrapper(CppEngineBot, "CheckMe (C++)")

    elif choice == "3":
        player1 = HumanPlayer("You")
        player2 = SecureBotWrapper(SmartBot, "SmartBot (Python)")

    elif choice == "5":
        player1 = HumanPlayer("Player 1")
        player2 = HumanPlayer("Player 2")

    else:
        # Default: Pacifist vs SmartBot
        player1 = SecureBotWrapper(PacifistBot, "Pacifist Bot")
        player2 = SecureBotWrapper(SmartBot, "SmartBot (Python)")

    ui = ChessUI(white_bot=player1, black_bot=player2)
    ui.run()