
import Math, Times, Streams, StrUtils

const width    = 1280
const height   = 720
const depthMax = 6
const fov      = 45.0

# ---------- ---------- ---------- #

type Vec3 =
  tuple
    x, y, z: float

# ---

{.push inline, noInit, noSideEffect.}

proc `-`(v:Vec3): Vec3 = (-v.x, -v.y, -v.z)

proc `+`(a, b:Vec3): Vec3 = (a.x + b.x, a.y + b.y, a.z + b.z)
proc `-`(a, b:Vec3): Vec3 = (a.x - b.x, a.y - b.y, a.z - b.z)

proc `*`(a, b:Vec3): Vec3 = (a.x * b.x, a.y * b.y, a.z * b.z)
proc `*`(a:Vec3, b:float): Vec3 = (a.x * b, a.y * b, a.z * b)

proc `/`(a, b:Vec3): Vec3 = (a.x / b.x, a.y / b.y, a.z / b.z)
proc `/`(a:Vec3, b:float): Vec3 = (a.x / b, a.y / b, a.z / b)

proc `+=`(this:var Vec3, v:Vec3) =
  this.x += v.x
  this.y += v.y
  this.z += v.z

{.pop.}

template zero(T:type Vec3): Vec3 = (0.0, 0.0, 0.0)

template dot(a, b:Vec3): float = (a.x * b.x) + (a.y * b.y) + (a.z * b.z)
template normalize(v:Vec3): Vec3 = v / sqrt(dot(v, v))
  
# ---------- ---------- ---------- #

type Ray =
  object
    pos, dir: Vec3

type Sphere =
  ref object
    pos: Vec3
    color: Vec3
    radius: float
    refl, tran: float

# ---

proc normal(this:Sphere, pos:Vec3): Vec3 {.inline, noInit.} =
  return normalize(pos - this.pos)


proc intersect(this:Sphere, ray:Ray): bool {.noInit.} =
  let d = this.pos - ray.pos
  let a = dot(d, ray.dir)
  if a < 0: # opposite direction
    return false

  let b2 = dot(d, d) - (a * a)
  let r2 = this.radius * this.radius
  if b2 > r2: # perpendicular > r
    return false
  
  return true


proc intersect(this:Sphere, ray:Ray, distance:var float): bool {.noInit.} =
  distance = 0
  
  let d = this.pos - ray.pos
  let a = dot(d, ray.dir)
  if a < 0: # opposite direction
    return false
  
  let b2 = dot(d, d) - (a * a)
  let r2 = this.radius * this.radius
  if b2 > r2: # perpendicular > r
    return false
  
  let c = sqrt(r2 - b2)
  let near = a - c
  let far  = a + c
  # near < 0 means ray starts inside
  distance = if near < 0: far else: near
  
  return true

# ---------- ---------- ---------- #

type Light =
  ref object
    pos, color: Vec3

type Scene =
  ref object
    objects: seq[Sphere]
    lights: seq[Light]

# ---

proc new(T:type Scene): Scene =
  new result
  result.objects = @[]
  result.lights = @[]

# ---

proc trace(this:Ray, scene:Scene, depth:int): Vec3 {.noInit.} = 
  var nearest = inf
  var obj: Sphere
  
  # search the scene for nearest intersection
  for o in scene.objects:
    var distance = inf
    if o.intersect(this, distance):
      if distance < nearest:
        nearest = distance
        obj = o
  
  if obj == nil:
    return Vec3.zero
  
  let hitPoint = this.pos + (this.dir * nearest)
  var normal = obj.normal(hitPoint)
  var inside = false
  
  if dot(normal, this.dir) > 0:
    inside = true
    normal = -normal
  
  var color = Vec3.zero
  let reflRatio = obj.refl
  
  for light in scene.lights:
    let lightDir = normalize(light.pos - hitPoint)
    let tr = Ray(pos:hitPoint + (normal * 1.0e-5), dir:lightDir)
    
    # go through the scene check whether we're blocked from the lights
    var blocked = false
    for o in scene.objects:
      if o.intersect(tr):
        blocked = true
        break
    
    if not blocked:
      color += light.color * max(0.0, dot(normal, lightDir)) *
        obj.color * (1.0 - reflRatio)
  
  let rayNormDot = dot(this.dir, normal)
  let facing = max(0.0, -rayNormDot)
  let fresnel = reflRatio + ((1.0 - reflRatio) * pow((1.0 - facing), 5.0))
  
  # compute reflection
  if depth < depthMax and reflRatio > 0:
    let reflDir = this.dir + (normal * 2 * rayNormDot * -1.0)
    let tr = Ray(pos:hitPoint + (normal * 1.0e-5), dir:reflDir)
    color += tr.trace(scene, depth + 1) * fresnel
  
  # compute refraction
  if depth < depthMax and obj.tran > 0:
    const iorDefault = 1.0 / 1.5
    let ior = if inside: iorDefault else: 1.5
    let ce = dot(this.dir, normal) * -1.0
    let eta = 1.0 / ior
    let gf = (this.dir + normal * ce) * eta
    let sin_t1_2 = 1.0 - (ce * ce)
    let sin_t2_2 = sin_t1_2 * (eta * eta)
    
    if sin_t2_2 < 1.0:
      let gc = normal * sqrt(1.0 - sin_t2_2)
      let refrDir = gf - gc
      let tr = Ray(pos:hitPoint - (normal * 1.0e-4), dir:refrDir)
      color += tr.trace(scene, depth + 1) * (1.0 - fresnel) * obj.tran
  
  return color

# ---------- ---------- ---------- #

const pixmapSize = width * height * 3
type Pixmap = array[pixmapSize, byte]

# ---

proc render(this:ref Pixmap, scene:Scene) =
  let h = tan(((fov / 360.0) * (2.0 * Pi)) / 2.0) * 2.0
  let w = h * width / height
  
  for y in 0 .. height-1:
    for x in 0 .. width-1:
      let wf: float = width
      let hf: float = height
      let dir: Vec3 = (
        x: ((float(x) - (wf / 2.0)) / wf) * w,
        y: (((hf / 2.0) - float(y)) / hf) * h,
        z: -1.0
      ).normalize()
      
      let ray = Ray(pos:Vec3.zero, dir:dir)
      let pixel = ray.trace(scene, 0)
      let index: int = (x * 3) + (y * width * 3)
      
      this[index]   = min(pixel.x * 255, 255).byte
      this[index+1] = min(pixel.y * 255, 255).byte
      this[index+2] = min(pixel.z * 255, 255).byte

# ---------- ---------- ---------- #

proc main =
  ## raytraces a simple scene and saves an image
  let scene = new Scene
  let image = new Pixmap
  
  # add geometry
  scene.objects.add Sphere(pos:(0.0, -10002.0, -20.0), radius:10000, color:(0.8, 0.8, 0.8))
  scene.objects.add Sphere(pos:( 0.0,  2.0, -20.0), radius:4, color:(0.8, 0.5, 0.5), refl:0.5)
  scene.objects.add Sphere(pos:( 5.0,  0.0, -15.0), radius:2, color:(0.3, 0.8, 0.8), refl:0.2)
  scene.objects.add Sphere(pos:(-5.0,  0.0, -15.0), radius:2, color:(0.3, 0.5, 0.8), refl:0.2)
  scene.objects.add Sphere(pos:(-2.0, -1.0, -10.0), radius:1, color:(0.1, 0.1, 0.1), refl:0.1, tran:0.8)
  
  # add light
  scene.lights.add Light (
    pos:(-10.0, 20.0, 30.0),
    color:(2.0, 2.0, 2.0)
  )
  
  # run test
  echo "Starting test..."
  
  let start = (cpuTime() * 1000.0)
  image.render(scene)
  let duration = (cpuTime() * 1000.0) - start
  
  echo "Mil: ", duration.formatFloat(FFDecimal, 1)
  echo "Sec: ", (duration / 1000.0).formatFloat(FFDecimal, 2)
  
  # save image
  let fs = newFileStream("Image.raw", FMWrite)
  fs.writeData(cast[pointer](image), pixmapSize)

# ---

when isMainModule:
  main() # run program
