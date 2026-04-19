class Player:
    def __init__(self, hp=100, row=0, potions=3):
        self.hp = hp
        self.row = row
        self.potions = potions

    def move_up(self):
        self.row -= 1

    def move_down(self):
        self.row += 1

    def shoot(self, enemy_row):
        return "SHOOT"

    def heal(self):
        if self.potions > 0:
            self.potions -= 1
            self.hp += 30
            return "HEAL"
        return None
