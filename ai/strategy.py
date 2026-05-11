import random
from collections import defaultdict

class MarkovStrategy:
    def __init__(self):
        # transition matrix: row -> next_row -> count
        self.transitions = defaultdict(lambda: defaultdict(int))
        self.last_row = None

    def update(self, prev_row, current_row):
        if prev_row is not None:
            self.transitions[prev_row][current_row] += 1

    def predict_distribution(self, current_row):
        """Return probability distribution of next rows"""
        next_states = self.transitions[current_row]

        if not next_states:
            return {current_row: 1.0}

        total = sum(next_states.values())

        return {
            row: count / total
            for row, count in next_states.items()
        }

    def sample_next_row(self, distribution):
        """Stochastic prediction (important Markov behavior)"""
        r = random.random()
        cumulative = 0.0

        for row, prob in distribution.items():
            cumulative += prob
            if r <= cumulative:
                return row

        return list(distribution.keys())[0]

    def decide(self, player, enemy):
        # Update model
        if self.last_row is not None:
            self.update(self.last_row, enemy.row)

        self.last_row = enemy.row

        # Predict distribution
        dist = self.predict_distribution(enemy.row)

        predicted_row = self.sample_next_row(dist)

        # Decision logic using probability
        if player.hp < 30 and player.potions > 0:
            return "HEAL"

        if player.row == predicted_row:
            return "SHOOT"

        # move toward most likely position
        if player.row < predicted_row:
            return "DOWN"
        else:
            return "UP"