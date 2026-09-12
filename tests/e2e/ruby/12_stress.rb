# e2e-markers: class Game, def turn, case action, begin
class Game
  def initialize(players)
    @players = players
    @round = 0
  end

  def turn(player)
    @round += 1
    begin
      action = player.choose
      case action
      when :attack then strike(player)
      when :defend
        guard(player)
      else
        rest(player)
      end
    rescue GameOver => e
      announce(e)
    ensure
      cleanup(player)
    end
  end

  def each_player
    @players.each do |p|
      yield p
    end
  end
end
