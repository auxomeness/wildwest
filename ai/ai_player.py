from base_player import Player
from strategy import MarkovStrategy

class AIPlayer(Player):
    def __init__(self, strategy=None):
        super().__init__()
        self.strategy = strategy if strategy else MarkovStrategy()

    def decide_action(self, enemy):
        return self.strategy.decide(self, enemy)
