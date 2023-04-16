import com.daveclay.aunisoma.audio.Audio
import com.daveclay.aunisoma.musictheory.Progression
import processing.core.PApplet

fun main(args: Array<String>) {
    PApplet.main(MySketch::class.java, "")
}

class MySketch : PApplet() {
    private val audio = Audio()

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

        val progression = Progression.i_iii_IV_V
        val scalePositionIndex = interpolateMouseWidth(mouseXf, progression.scalePositions.size)
        val scalePosition = progression.scalePositions[scalePositionIndex];

        audio.scalePosition = scalePosition
        val keyChord = audio.update()

        val notesText = "Notes: ${keyChord.notes.map { note -> note.name }.joinToString(", ")}"
        text("Chord: ${keyChord.name} - $notesText", 100f, 120f);
    }
}