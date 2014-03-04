#! /usr/bin/env ruby
#
# usage: from_scratch.rb
#
# This script shows you how to create a new sequence from scratch and save it
# to a MIDI file. It creates a file called 'from_scratch.mid'.

# Start looking for MIDI module classes in the directory above this one.
# This forces us to use the local copy, even if there is a previously
# installed version out there somewhere.
$LOAD_PATH[0, 0] = File.join(File.dirname(__FILE__), '..', 'lib')

require 'midilib/sequence'
require 'midilib/consts'
include MIDI


C  = 0
Cs = 1
D  = 2
Ds = 3
E  = 4
Es = 5
F  = 6
Fs = 7
G  = 8
Gs = 9
A  = 10
As = 11
B  = 12

state_table = [
   [C, Cs], #C
   [Cs, D], #Cs
   [D, Ds], #D
   [Ds, E], #Ds
   [E, Es], #E
   [Es, F], #Es
   [F, Fs], #F
   [Fs, G], #Fs
   [G, Gs], #G
   [Gs, A], #Gs
   [A, As], #A
   [As, B], #As
   [B, C]   #B
]

seq = Sequence.new()

# Create a first track for the sequence. This holds tempo events and stuff
# like that.
track = Track.new(seq)
seq.tracks << track
track.events << Tempo.new(Tempo.bpm_to_mpq(120))
track.events << MetaEvent.new(META_SEQ_NAME, 'Sequence Name')

# Create a track to hold the notes. Add it to the sequence.
track = Track.new(seq)
seq.tracks << track

# Give the track a name and an instrument name (optional).
track.name = 'My New Track'
track.instrument = GM_PATCH_NAMES[0]

# Add a volume controller event (optional).
track.events << Controller.new(0, CC_VOLUME, 127)

# Add events to the track: a major scale. Arguments for note on and note off
# constructors are channel, note, velocity, and delta_time. Channel numbers
# start at zero. We use the new Sequence#note_to_delta method to get the
# delta time length of a single quarter note.
track.events << ProgramChange.new(0, 1, 0)
quarter_note_length = seq.note_to_delta('quarter')

state = C

for i in 0..40
   track.events << NoteOn.new(0, 64 + state, 127, 0)
   track.events << NoteOff.new(0, 64 + state, 127, quarter_note_length)
   possible_states = state_table[state]

   index = 0 + rand(possible_states.count)
   puts(index)
   state = possible_states[index]
end

#[0, 2, 4, 5, 7, 9, 11, 12].each do |offset|
#  track.events << NoteOn.new(0, 64 + offset, 127, 0)
#  track.events << NoteOff.new(0, 64 + offset, 127, quarter_note_length)
#end


# Calling recalc_times is not necessary, because that only sets the events'
# start times, which are not written out to the MIDI file. The delta times are
# what get written out.

# track.recalc_times

File.open('from_scratch.mid', 'wb') { |file| seq.write(file) }