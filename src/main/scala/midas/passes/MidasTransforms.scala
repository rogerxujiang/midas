package midas
package passes

import midas.core._
import firrtl._
import firrtl.ir._
import firrtl.Mappers._
import firrtl.annotations._
import Utils._
import java.io.{File, FileWriter}

private class WCircuit(
  info: Info,
  modules: Seq[DefModule],
  main: String,
  val sim: SimWrapperIO) extends Circuit(info, modules, main)

object MidasAnnotation {
  def apply(t: String, conf: File, json: File, lib: Option[File]) =
    Annotation(CircuitName(t), classOf[MidasTransforms],
               s"$conf $json %s".format(lib map (_.toString) getOrElse ""))
  private val matcher = "([^ ]+) ([^ ]+) ([^ ]*)".r
  def unapply(a: Annotation) = a match {
    case Annotation(CircuitName(c), t, matcher(conf, json, lib))
      if t == classOf[MidasTransforms] =>
        Some(c, new File(conf), new File(json), if (lib.isEmpty) None else Some(new File(lib)))
    case _ => None
  }
}

class DeleteDebugLogic(noDebug: Boolean = true) extends firrtl.passes.Pass {
  override def inputForm = LowForm
  override def outputForm = LowForm

  def onStmt(s: Statement): Statement =
    s map onStmt match {
      case s @ (_: Stop | _: Print) => EmptyStmt
      case s => s
    }

  def run(c: Circuit) = c.copy(modules = c.modules map (_ map onStmt))
}

private[midas] class MidasTransforms(
    dir: File,
    io: chisel3.Data)
   (implicit param: config.Parameters) extends Transform {
  def inputForm = LowForm
  def outputForm = LowForm
  def execute(state: CircuitState) = (getMyAnnotations(state): @unchecked) match {
    case Seq(MidasAnnotation(state.circuit.main, conf, json, lib)) =>
      val xforms = Seq(
        new DeleteDebugLogic(param(NoDebug)),
        firrtl.passes.RemoveValidIf,
        new firrtl.transforms.ConstantPropagation,
        firrtl.passes.SplitExpressions,
        firrtl.passes.CommonSubexpressionElimination,
        new firrtl.transforms.DeadCodeElimination,
        new ConfToJSON(conf, json),
        new barstools.macros.MacroCompilerTransform,
        firrtl.passes.ResolveKinds,
        firrtl.passes.RemoveEmpty,
        new Fame1Transform(lib getOrElse json),
        new strober.passes.StroberTransforms(dir, lib getOrElse json),
        new SimulationMapping(io),
        new PlatformMapping(state.circuit.main, dir))
      (xforms foldLeft state)((in, xform) =>
        xform runTransform in).copy(form=outputForm)
  }
}
