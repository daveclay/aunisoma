import musictheory.ChordType
import musictheory.Note
import musictheory.Note.Companion.NOTES
import musictheory.Progression
import musictheory.Scale
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


class AudioThing(audioContext: AudioContext,
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

class AudioThingFactory(private val audioContext: AudioContext,
                        private val mixer: ScalingMixer) {
    fun newAudioThing() = AudioThing(audioContext, mixer)
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

    private val audioThingFactory = AudioThingFactory(audioContext, mixer)

    private val audioThing1 = audioThingFactory.newAudioThing()
    private val audioThing2 = audioThingFactory.newAudioThing()
    private val audioThing3 = audioThingFactory.newAudioThing()
    // private val audioThing4 = audioThingFactory.newAudioThing()

    private val audioThings = arrayOf(
        audioThing1,
        audioThing2,
        audioThing3,
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

        audioThing1.setFrequency(Note.C.frequencyAtOctave(3))
        audioThing2.setFrequency(Note.E.frequencyAtOctave(3))
        audioThing3.setFrequency(Note.G.frequencyAtOctave(3))
        // audioThing4.setFrequency(Note.C.frequencyAtOctave(4))
    }

    fun setGain(gain: Float) {
        gainGlide.value = gain
    }

    fun setFrequency(freq: Float) {
        audioThing1.setFrequency(freq)
    }

    fun setNotes(notes: Collection<Note>) {
        notes.withIndex().forEach { (index, note) ->
            if (index < audioThings.size) {
                val audioThing = audioThings[index]
                audioThing.setFrequency(note.frequencyAtOctave(3))
            }
        }
    }
}

class MySketch : PApplet() {
    private val audio = Audio()

    val progression = Progression.I_iii_IV
    val scale = Scale.MAJOR_SCALE
    val rootNote = Note.C

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

        val scaleIndiciesIndex = interpolateMouseWidth(mouseXf, progression.scaleIndicies.size)
        val scaleIndexToStart = progression.scaleIndicies.toList().get(scaleIndiciesIndex);

        val chordNotes = scale.buildChordNotes(rootNote, scaleIndexToStart, ChordType.TRIAD_CHORD)
        audio.setNotes(chordNotes)

        text("Notes: ${chordNotes.map { note -> note.name }.joinToString(", ")}", 100f, 120f);
    }
}