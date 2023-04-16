import musictheory.*
import net.beadsproject.beads.core.AudioContext
import net.beadsproject.beads.data.Buffer
import net.beadsproject.beads.ugens.Gain
import net.beadsproject.beads.ugens.Glide
import net.beadsproject.beads.ugens.ScalingMixer
import net.beadsproject.beads.ugens.WavePlayer
import processing.core.PApplet

fun main(args: Array<String>) {
    PApplet.main(MySketch::class.java, "")
}


class Voice(audioContext: AudioContext,
            mixer: ScalingMixer) {
    private val frequencyGlide = Glide(audioContext,
        440f,
        0f);

    private val wavePlayer = WavePlayer(
        audioContext,
        frequencyGlide,
        Buffer.SINE)

    private val gainGlide = Glide(
        audioContext,
        0.5f,
        0f);

    private val gain = Gain(
        audioContext,
        1,
        gainGlide
    )

    init {
        gain.addInput(wavePlayer)
        mixer.addInput(gain)
    }

    fun setFrequency(frequency: Float) {
        frequencyGlide.value = frequency
    }
}

class VoiceBuilder(private val audioContext: AudioContext,
                   private val mixer: ScalingMixer) {
    fun buildNewVoice() = Voice(audioContext, mixer)
}

/**
 * https://manualzz.com/doc/23754747/sonifying-processing--the-beads-tutorial
 */
class Audio {
    private val audioContext = AudioContext()
    private val mixer = ScalingMixer(audioContext)
    private val gainGlide = Glide(
        audioContext,
        0.5f,
        0f);
    private val gain = Gain(
        audioContext,
        1,
        gainGlide
    )

    private val voiceBuilder = VoiceBuilder(audioContext, mixer)

    private val voice1 = voiceBuilder.buildNewVoice()
    private val voice2 = voiceBuilder.buildNewVoice()
    private val voice3 = voiceBuilder.buildNewVoice()

    private val voices = arrayOf(
        voice1,
        voice2,
        voice3,
    )

    /*
     * A.next().next().next()
     * A.next(chord).next(chord).next(chord)
     * A.next(progression)* .next(progression)
     *
     * context.add(perspective): 12 perspectives? or one per person? Fundamentals, adding mults
     */

    fun initialize() {
        gain.addInput(mixer)
        audioContext.out.addInput(gain)
        audioContext.start()

        voice1.setFrequency(Note.C.frequencyAtOctave(3))
        voice2.setFrequency(Note.E.frequencyAtOctave(3))
        voice3.setFrequency(Note.G.frequencyAtOctave(3))
        // audioThing4.setFrequency(Note.C.frequencyAtOctave(4))
    }

    fun setNotes(notes: List<Note>) {
        val prevFreq = 0 // open up the chord voicing by going an octave up instead of all notes in the same octave.
        val baseOctave = 2

        notes.withIndex().forEach { (index, note) ->
            if (index < voices.size) {
                val audioThing = voices[index]
                val frequencyAtOctave = note.frequencyAtOctave(baseOctave)
                if (frequencyAtOctave < prevFreq) {
                    val up = note.frequencyAtOctave(baseOctave + 1)
                    audioThing.setFrequency(up)
                } else {
                    audioThing.setFrequency(frequencyAtOctave)
                }
            }
        }
    }
}

class MySketch : PApplet() {
    private val audio = Audio()

    val progression = Progression.i_iii_IV_V
    val scale = Scale.MINOR_SCALE
    val rootNote = Note.C
    val musicalKey = Key(rootNote, scale)

    override fun settings() {
        size(500, 500)
    }

    override fun setup() {
        audio.initialize()
    }

    fun interpolateMouseWidth(mouseX: Float, size: Int): Int {
        return ((mouseX / width.toFloat()) * size).toInt()
    }

    override fun draw() {
        background(64)
        val mouseXf = mouseX.toFloat()
        val mouseYf = mouseY.toFloat()

        ellipse(mouseXf, mouseYf, 20f, 20f)

        val scalePositionIndex = interpolateMouseWidth(mouseXf, progression.scalePositions.size)
        val scalePosition = progression.scalePositions[scalePositionIndex];

        // Combine a Progression with a ChordType for each index in a Scale - play different chords per scale index.
        val keyChord = musicalKey.getChord(ChordType.TRIAD_CHORD, scalePosition)
        val notes = keyChord.notes

        audio.setNotes(notes)

        val notesText = "Notes: ${notes.map { note -> note.name }.joinToString(", ")}"
        text("Chord: ${keyChord.name} - $notesText", 100f, 120f);
    }
}