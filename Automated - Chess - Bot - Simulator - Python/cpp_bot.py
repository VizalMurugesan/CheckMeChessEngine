# cpp_bot.py
import subprocess
import os
from pathlib import Path
import chess
from chess_player import ChessPlayer

DEFAULT_ENGINE_PATH = Path(__file__).resolve().parent.parent / "ChessEngine-CPP" / "build" / "Debug" / "ChessEngine.exe"

class CppEngineBot(ChessPlayer):
    def __init__(self, name="CheckMe C++", engine_path=None):
        super().__init__(name)
        target_path = Path(engine_path) if engine_path else DEFAULT_ENGINE_PATH
        self.engine_path = str(target_path.resolve())

        if not os.path.exists(self.engine_path):
            raise FileNotFoundError(f"Engine executable not found at: {self.engine_path}")

        self.process = subprocess.Popen(
            [self.engine_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )

        self._send("uci")
        while True:
            if self._read() == "uciok":
                break

        self._send("isready")
        while True:
            if self._read() == "readyok":
                break

    def _send(self, command: str):
        self.process.stdin.write(command + "\n")
        self.process.stdin.flush()

    def _read(self) -> str:
        return self.process.stdout.readline().strip()

    def make_move(self, board: chess.Board):
        if board.is_game_over():
            return None

        fen = board.fen()
        self._send(f"position fen {fen}")
        self._send("go")

        while True:
            line = self._read()
            if line.startswith("bestmove"):
                parts = line.split()
                if len(parts) >= 2 and parts[1] != "none":
                    move_str = parts[1]
                    try:
                        return chess.Move.from_uci(move_str)
                    except ValueError:
                        return list(board.legal_moves)[0] if board.legal_moves else None
                return None

    def close(self):
        if hasattr(self, "process") and self.process and self.process.poll() is None:
            try:
                self._send("quit")
                self.process.communicate(timeout=1)
            except Exception:
                self.process.kill()

    def __del__(self):
        self.close()