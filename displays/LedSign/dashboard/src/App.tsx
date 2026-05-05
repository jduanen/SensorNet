import { useState, useEffect, useRef, useCallback } from "react";

// ── Types ──────────────────────────────────────────────────────────────────
type ColorKey = "Red" | "Green" | "Amber" | "Off";
type FontKey = 0 | 1 | 2 | 3 | 4;

interface Segment {
  id: number;
  color: ColorKey;
  font: FontKey;
  text: string;
}

// ── Constants ──────────────────────────────────────────────────────────────
const haEntity = "input_text.led_display_text_input";

const colorMap: Record<ColorKey, { dot: string }> = {
  Red:   { dot: "#ef4444" },
  Green: { dot: "#22c55e" },
  Amber: { dot: "#f59e0b" },
  Off:   { dot: "#4b5563" },
};

const colorCodesNum: Record<ColorKey, number> = {
  Off: 0, Red: 1, Green: 2, Amber: 3,
};

const numToColor: Record<number, ColorKey> = {
  0: "Off", 1: "Red", 2: "Green", 3: "Amber",
};

const fontCodes: Record<FontKey, string> = {
  0: "MT Pixel 5x7",
  1: "MatrixLight6",
  2: "MatrixLight6X",
  3: "MatrixLight8",
  4: "Roboto",
};

// ── String encoding / decoding ─────────────────────────────────────────────
// Format: \[<fontNum>;<colorNum>m<text>  e.g. \[0;1mHello \[0;2mWorld
const buildFullString = (segments: Segment[]): string =>
  segments.map((s) => `\\[${s.font};${colorCodesNum[s.color]}m${s.text}`).join("");

let _segId = 0;
const newSegId = () => ++_segId;

function parseEscapeString(raw: string): Segment[] {
  const re = /\\\[(\d+);(\d+)m([^\\]*)/g;
  const segments: Segment[] = [];
  let match: RegExpExecArray | null;
  while ((match = re.exec(raw)) !== null) {
    const fontNum = parseInt(match[1]);
    const colorNum = parseInt(match[2]);
    const text = match[3];
    const font = (fontNum >= 0 && fontNum <= 4 ? fontNum : 0) as FontKey;
    const color: ColorKey = numToColor[colorNum] ?? "Amber";
    if (text.length > 0) {
      segments.push({ id: newSegId(), color, font, text });
    }
  }
  // Fallback: plain text with no escape codes
  if (segments.length === 0 && raw.trim()) {
    segments.push({ id: newSegId(), color: "Amber", font: 0, text: raw });
  }
  return segments;
}

// ── Presets ────────────────────────────────────────────────────────────────
const presets: { label: string; dot: string; segments: Omit<Segment, "id">[] }[] = [
  {
    label: "Welcome Home",
    dot: "#f59e0b",
    segments: [
      { color: "Amber", font: 0, text: "Welcome " },
      { color: "Green", font: 0, text: "Home!" },
    ],
  },
  {
    label: "Do Not Forget",
    dot: "#ef4444",
    segments: [
      { color: "Amber", font: 0, text: "Do Not " },
      { color: "Red",   font: 0, text: "Forget!" },
    ],
  },
  {
    label: "Warning",
    dot: "#ef4444",
    segments: [{ color: "Red", font: 0, text: "WARNING" }],
  },
  {
    label: "Check Garage",
    dot: "#f59e0b",
    segments: [
      { color: "Green", font: 0, text: "Remember: " },
      { color: "Amber", font: 0, text: "Check garage door!" },
    ],
  },
  {
    label: "Color Demo",
    dot: "#f59e0b",
    segments: [
      { color: "Red",   font: 0, text: "RED," },
      { color: "Green", font: 0, text: "GREEN," },
      { color: "Amber", font: 0, text: "AMBER!" },
    ],
  },
  {
    label: "Font Demo",
    dot: "#22c55e",
    segments: [
      { color: "Green", font: 0, text: "F0 " },
      { color: "Green", font: 1, text: "F1 " },
      { color: "Green", font: 2, text: "F2" },
    ],
  },
];

// ── 5×7 Bitmap Font ───────────────────────────────────────────────────────
// Standard LED-matrix 5×7 font, ASCII 32–126.
// Each entry: [col0..col4] where bit0 = top row, bit6 = bottom row.
const font5x7: readonly [number,number,number,number,number][] = [
  [0x00,0x00,0x00,0x00,0x00], // ' ' 32
  [0x00,0x00,0x5F,0x00,0x00], // '!' 33
  [0x00,0x07,0x00,0x07,0x00], // '"' 34
  [0x14,0x7F,0x14,0x7F,0x14], // '#' 35
  [0x24,0x2A,0x7F,0x2A,0x12], // '$' 36
  [0x23,0x13,0x08,0x64,0x62], // '%' 37
  [0x36,0x49,0x55,0x22,0x50], // '&' 38
  [0x00,0x05,0x03,0x00,0x00], // ''' 39
  [0x00,0x1C,0x22,0x41,0x00], // '(' 40
  [0x00,0x41,0x22,0x1C,0x00], // ')' 41
  [0x14,0x08,0x3E,0x08,0x14], // '*' 42
  [0x08,0x08,0x3E,0x08,0x08], // '+' 43
  [0x00,0x50,0x30,0x00,0x00], // ',' 44
  [0x08,0x08,0x08,0x08,0x08], // '-' 45
  [0x00,0x60,0x60,0x00,0x00], // '.' 46
  [0x20,0x10,0x08,0x04,0x02], // '/' 47
  [0x3E,0x51,0x49,0x45,0x3E], // '0' 48
  [0x00,0x42,0x7F,0x40,0x00], // '1' 49
  [0x42,0x61,0x51,0x49,0x46], // '2' 50
  [0x21,0x41,0x45,0x4B,0x31], // '3' 51
  [0x18,0x14,0x12,0x7F,0x10], // '4' 52
  [0x27,0x45,0x45,0x45,0x39], // '5' 53
  [0x3C,0x4A,0x49,0x49,0x30], // '6' 54
  [0x01,0x71,0x09,0x05,0x03], // '7' 55
  [0x36,0x49,0x49,0x49,0x36], // '8' 56
  [0x06,0x49,0x49,0x29,0x1E], // '9' 57
  [0x00,0x36,0x36,0x00,0x00], // ':' 58
  [0x00,0x56,0x36,0x00,0x00], // ';' 59
  [0x08,0x14,0x22,0x41,0x00], // '<' 60
  [0x14,0x14,0x14,0x14,0x14], // '=' 61
  [0x00,0x41,0x22,0x14,0x08], // '>' 62
  [0x02,0x01,0x51,0x09,0x06], // '?' 63
  [0x32,0x49,0x79,0x41,0x3E], // '@' 64
  [0x7E,0x11,0x11,0x11,0x7E], // 'A' 65
  [0x7F,0x49,0x49,0x49,0x36], // 'B' 66
  [0x3E,0x41,0x41,0x41,0x22], // 'C' 67
  [0x7F,0x41,0x41,0x22,0x1C], // 'D' 68
  [0x7F,0x49,0x49,0x49,0x41], // 'E' 69
  [0x7F,0x09,0x09,0x09,0x01], // 'F' 70
  [0x3E,0x41,0x49,0x49,0x7A], // 'G' 71
  [0x7F,0x08,0x08,0x08,0x7F], // 'H' 72
  [0x00,0x41,0x7F,0x41,0x00], // 'I' 73
  [0x20,0x40,0x41,0x3F,0x01], // 'J' 74
  [0x7F,0x08,0x14,0x22,0x41], // 'K' 75
  [0x7F,0x40,0x40,0x40,0x40], // 'L' 76
  [0x7F,0x02,0x0C,0x02,0x7F], // 'M' 77
  [0x7F,0x04,0x08,0x10,0x7F], // 'N' 78
  [0x3E,0x41,0x41,0x41,0x3E], // 'O' 79
  [0x7F,0x09,0x09,0x09,0x06], // 'P' 80
  [0x3E,0x41,0x51,0x21,0x5E], // 'Q' 81
  [0x7F,0x09,0x19,0x29,0x46], // 'R' 82
  [0x46,0x49,0x49,0x49,0x31], // 'S' 83
  [0x01,0x01,0x7F,0x01,0x01], // 'T' 84
  [0x3F,0x40,0x40,0x40,0x3F], // 'U' 85
  [0x1F,0x20,0x40,0x20,0x1F], // 'V' 86
  [0x3F,0x40,0x38,0x40,0x3F], // 'W' 87
  [0x63,0x14,0x08,0x14,0x63], // 'X' 88
  [0x07,0x08,0x70,0x08,0x07], // 'Y' 89
  [0x61,0x51,0x49,0x45,0x43], // 'Z' 90
  [0x00,0x7F,0x41,0x41,0x00], // '[' 91
  [0x02,0x04,0x08,0x10,0x20], // '\' 92
  [0x00,0x41,0x41,0x7F,0x00], // ']' 93
  [0x04,0x02,0x01,0x02,0x04], // '^' 94
  [0x40,0x40,0x40,0x40,0x40], // '_' 95
  [0x00,0x01,0x02,0x04,0x00], // '`' 96
  [0x20,0x54,0x54,0x54,0x78], // 'a' 97
  [0x7F,0x48,0x44,0x44,0x38], // 'b' 98
  [0x38,0x44,0x44,0x44,0x20], // 'c' 99
  [0x38,0x44,0x44,0x48,0x7F], // 'd' 100
  [0x38,0x54,0x54,0x54,0x18], // 'e' 101
  [0x08,0x7E,0x09,0x01,0x02], // 'f' 102
  [0x0C,0x52,0x52,0x52,0x3E], // 'g' 103
  [0x7F,0x08,0x04,0x04,0x78], // 'h' 104
  [0x00,0x44,0x7D,0x40,0x00], // 'i' 105
  [0x20,0x40,0x44,0x3D,0x00], // 'j' 106
  [0x7F,0x10,0x28,0x44,0x00], // 'k' 107
  [0x00,0x41,0x7F,0x40,0x00], // 'l' 108
  [0x7C,0x04,0x18,0x04,0x78], // 'm' 109
  [0x7C,0x08,0x04,0x04,0x78], // 'n' 110
  [0x38,0x44,0x44,0x44,0x38], // 'o' 111
  [0x7C,0x14,0x14,0x14,0x08], // 'p' 112
  [0x08,0x14,0x14,0x18,0x7C], // 'q' 113
  [0x7C,0x08,0x04,0x04,0x08], // 'r' 114
  [0x48,0x54,0x54,0x54,0x20], // 's' 115
  [0x04,0x3F,0x44,0x40,0x20], // 't' 116
  [0x3C,0x40,0x40,0x40,0x3C], // 'u' 117
  [0x1C,0x20,0x40,0x20,0x1C], // 'v' 118
  [0x3C,0x40,0x30,0x40,0x3C], // 'w' 119
  [0x44,0x28,0x10,0x28,0x44], // 'x' 120
  [0x0C,0x50,0x50,0x50,0x3C], // 'y' 121
  [0x44,0x64,0x54,0x4C,0x44], // 'z' 122
  [0x00,0x08,0x36,0x41,0x00], // '{' 123
  [0x00,0x00,0x7F,0x00,0x00], // '|' 124
  [0x00,0x41,0x36,0x08,0x00], // '}' 125
  [0x0C,0x02,0x01,0x02,0x0C], // '~' 126
];

// ── LED Canvas Preview ─────────────────────────────────────────────────────
// Physical display: 85 columns × 7 rows (17 chars × 5px each)
const ledCols = 85;
const ledRows = 7;
const dotSize = 7;
const dotGap = 2;

function LedPreview({
  text,
  scrollSpeed,
  isPaused,
}: {
  text: string;
  scrollSpeed: number;
  isPaused: boolean;
}) {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  const offsetRef = useRef(0);
  const rafRef = useRef<number>(0);
  const lastTimeRef = useRef<number>(0);

  // Parse escape-coded string into per-character color runs
  const parseSegments = useCallback((raw: string) => {
    const result: { char: string; color: string }[] = [];
    const colorHex: Record<number, string> = {
      0: "#111111", 1: "#ef4444", 2: "#22c55e", 3: "#f59e0b",
    };
    const re = /\\\[(\d+);(\d+)m([^\\]*)/g;
    const segs: { text: string; color: string }[] = [];
    let match: RegExpExecArray | null;
    while ((match = re.exec(raw)) !== null) {
      segs.push({ text: match[3], color: colorHex[parseInt(match[2])] ?? "#f59e0b" });
    }
    const source = segs.length > 0 ? segs : [{ text: raw, color: "#f59e0b" }];
    for (const seg of source)
      for (const c of seg.text) result.push({ char: c, color: seg.color });
    return result;
  }, []);

  // Build pixel buffer from char list using the 5×7 bitmap font.
  // Returns a 2D array [row][col] = hex color string, or null for off pixels.
  // Each character occupies 5 pixel columns + 1 gap column = 6px total.
  // Text starts at x = ledCols so it scrolls in from the right.
  const buildPixelBuf = useCallback(
    (chars: { char: string; color: string }[], bufW: number) => {
      const buf: (string | null)[][] = Array.from(
        { length: ledRows },
        () => new Array<string | null>(bufW).fill(null)
      );
      let xPos = ledCols;
      for (const { char, color } of chars) {
        const glyphIdx = char.charCodeAt(0) - 32;
        if (glyphIdx < 0 || glyphIdx >= font5x7.length) { xPos += 6; continue; }
        const glyph = font5x7[glyphIdx];
        for (let c = 0; c < 5; c++) {
          for (let r = 0; r < ledRows; r++) {
            if ((glyph[c] >> r) & 1) {
              const x = xPos + c;
              if (x >= 0 && x < bufW) buf[r][x] = color;
            }
          }
        }
        xPos += 6;
      }
      return buf;
    },
    []
  );

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext("2d");
    if (!ctx) return;

    // Reset scroll position when text changes
    offsetRef.current = 0;
    lastTimeRef.current = 0;

    canvas.width  = ledCols * (dotSize + dotGap);
    canvas.height = ledRows * (dotSize + dotGap);

    const chars = parseSegments(text);
    const bufW  = Math.max(ledCols, chars.length * 6 + ledCols);
    const pixelBuf = buildPixelBuf(chars, bufW);

    const draw = (timestamp: number) => {
      if (!isPaused) {
        if (lastTimeRef.current === 0) lastTimeRef.current = timestamp;
        if (timestamp - lastTimeRef.current >= scrollSpeed) {
          offsetRef.current = (offsetRef.current + 1) % bufW;
          lastTimeRef.current = timestamp;
        }
      } else {
        lastTimeRef.current = 0;
      }

      ctx.fillStyle = "#050a05";
      ctx.fillRect(0, 0, canvas.width, canvas.height);

      for (let row = 0; row < ledRows; row++) {
        for (let col = 0; col < ledCols; col++) {
          const cx = col * (dotSize + dotGap) + dotSize / 2;
          const cy = row * (dotSize + dotGap) + dotSize / 2;
          const lit = pixelBuf[row][(col + offsetRef.current) % bufW];

          ctx.beginPath();
          ctx.arc(cx, cy, dotSize / 2, 0, Math.PI * 2);
          if (lit) {
            ctx.fillStyle = lit;
            ctx.shadowColor = lit;
            ctx.shadowBlur = 3;
            ctx.fill();
            ctx.shadowBlur = 0;
          } else {
            ctx.fillStyle = "#0a180a";
            ctx.fill();
          }
        }
      }

      rafRef.current = requestAnimationFrame(draw);
    };

    rafRef.current = requestAnimationFrame(draw);
    return () => cancelAnimationFrame(rafRef.current);
  }, [text, scrollSpeed, isPaused, parseSegments, buildPixelBuf]);

  return (
    <canvas
      ref={canvasRef}
      style={{ width: "100%", height: "auto", borderRadius: "6px", display: "block" }}
    />
  );
}

// ── HA REST API helpers ────────────────────────────────────────────────────
// Returns the scheme+host+port of a URL, or "" if the input isn't a valid
// absolute HTTP/HTTPS URL. Never returns a relative path fragment.
function haOrigin(haUrl: string): string {
  try {
    const o = new URL(haUrl).origin;
    if (o && o !== "null" && (o.startsWith("http://") || o.startsWith("https://"))) return o;
  } catch { /* fall through */ }
  return "";
}

async function haSetValue(
  haUrl: string,
  haToken: string,
  value: string
): Promise<{ ok: boolean; message: string }> {
  const base = haOrigin(haUrl);
  if (!base) return { ok: false, message: "Invalid HA URL — open Settings and enter the full URL (e.g. http://homeassistant.local:8123)" };
  try {
    const res = await fetch(
      `${base}/api/services/input_text/set_value`,
      {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          Authorization: `Bearer ${haToken}`,
        },
        body: JSON.stringify({ entity_id: haEntity, value }),
      }
    );
    if (res.ok) return { ok: true, message: "Sent to LED Sign ✓" };
    const body = await res.text();
    return { ok: false, message: `HA Error ${res.status} (${base}): ${body}` };
  } catch (e: unknown) {
    return { ok: false, message: `Network error: ${(e as Error).message}` };
  }
}

async function haGetState(haUrl: string, haToken: string): Promise<string | null> {
  const base = haOrigin(haUrl);
  if (!base) return null;
  try {
    const res = await fetch(
      `${base}/api/states/${haEntity}`,
      { headers: { Authorization: `Bearer ${haToken}` } }
    );
    if (!res.ok) return null;
    const data = await res.json();
    return typeof data.state === "string" ? data.state : null;
  } catch {
    return null;
  }
}

// ── Main App ───────────────────────────────────────────────────────────────
export default function App() {
  const [segments, setSegments] = useState<Segment[]>([
    { id: newSegId(), color: "Red",   font: 0, text: "RED,"   },
    { id: newSegId(), color: "Green", font: 0, text: "GREEN," },
    { id: newSegId(), color: "Amber", font: 0, text: "AMBER!" },
  ]);

  const [scrollSpeed, setScrollSpeed] = useState(90);
  const [isPaused, setIsPaused] = useState(false);
  const [previewTab, setPreviewTab] = useState<"composing" | "live">("live");
  const [outputTab, setOutputTab] = useState<"plain" | "yaml" | "lambda">("plain");
  const [copyLabel, setCopyLabel] = useState("Copy");
  const [sendStatus, setSendStatus] = useState<{ ok: boolean; message: string } | null>(null);
  const [isSending, setIsSending] = useState(false);
  const [isLoading, setIsLoading] = useState(false);

  // Live HA state — polled independently of the composition
  const [liveHaString, setLiveHaString] = useState<string | null>(null);
  const [liveHaUpdated, setLiveHaUpdated] = useState<Date | null>(null);

  // HA config — persisted to localStorage
  const [haUrl, setHaUrl] = useState(() => {
    const saved = localStorage.getItem("ledSignHaUrl");
    if (saved) {
      const origin = haOrigin(saved);
      if (origin) return origin;
      localStorage.removeItem("ledSignHaUrl"); // bad value — drop it
    }
    const { protocol, hostname, port } = window.location;
    if (hostname !== "localhost" && hostname !== "127.0.0.1") {
      return `${protocol}//${hostname}${port ? ":" + port : ""}`;
    }
    return "http://homeassistant.local:8123";
  });
  const [haToken, setHaToken] = useState(
    () => localStorage.getItem("ledSignHaToken") ?? ""
  );
  const [showConfig, setShowConfig] = useState(false);

  const saveHaUrl = (val: string) => {
    setHaUrl(val); // allow free typing; origin is extracted at call time
    const origin = haOrigin(val);
    if (origin) localStorage.setItem("ledSignHaUrl", origin);
  };
  const saveHaToken = (val: string) => {
    setHaToken(val);
    localStorage.setItem("ledSignHaToken", val);
  };

  // Background poll — updates the live preview without touching the composition
  const pollHaState = useCallback(async () => {
    if (!haToken) return;
    const state = await haGetState(haUrl, haToken);
    if (state !== null) {
      setLiveHaString(state === "unknown" ? "" : state);
      setLiveHaUpdated(new Date());
    }
  }, [haUrl, haToken]);

  useEffect(() => {
    if (!haToken) return;
    pollHaState();
    const id = setInterval(pollHaState, 15_000);
    return () => clearInterval(id);
  }, [haToken, haUrl, pollHaState]);

  const displayString = buildFullString(segments);

  // ── Output formats ─────────────────────────────────────────────────────
  const yamlOutput =
    `service: input_text.set_value\n` +
    `target:\n  entity_id: ${haEntity}\n` +
    `data:\n  value: "${displayString}"`;
  const lambdaOutput = `id(led_display_text).publish_state("${displayString}");`;
  const currentOutput =
    outputTab === "plain" ? displayString :
    outputTab === "yaml"  ? yamlOutput : lambdaOutput;

  // ── Load current state from HA into the segment builder ───────────────
  const handleLoadFromHa = useCallback(async () => {
    if (!haToken) return;
    setIsLoading(true);
    const state = await haGetState(haUrl, haToken);
    setIsLoading(false);
    if (state !== null && state !== "unknown" && state !== "") {
      setLiveHaString(state);
      setLiveHaUpdated(new Date());
      const parsed = parseEscapeString(state);
      if (parsed.length > 0) {
        setSegments(parsed);
        setSendStatus({ ok: true, message: "Loaded current display state ✓" });
        setTimeout(() => setSendStatus(null), 3000);
        return;
      }
    }
    setSendStatus({ ok: false, message: "Could not load state (check URL & token)" });
    setTimeout(() => setSendStatus(null), 4000);
  }, [haUrl, haToken]);

  // ── Segment operations ─────────────────────────────────────────────────
  const addSegment = () =>
    setSegments((s) => [...s, { id: newSegId(), color: "Green", font: 0, text: "Text" }]);

  const removeSegment = (id: number) =>
    setSegments((s) => s.filter((seg) => seg.id !== id));

  const moveSegment = (id: number, dir: -1 | 1) =>
    setSegments((s) => {
      const idx = s.findIndex((seg) => seg.id === id);
      if (idx < 0) return s;
      const newIdx = idx + dir;
      if (newIdx < 0 || newIdx >= s.length) return s;
      const copy = [...s];
      [copy[idx], copy[newIdx]] = [copy[newIdx], copy[idx]];
      return copy;
    });

  const updateSegment = (id: number, patch: Partial<Omit<Segment, "id">>) =>
    setSegments((s) =>
      s.map((seg) => (seg.id === id ? { ...seg, ...patch } : seg))
    );

  const applyPreset = (preset: (typeof presets)[0]) =>
    setSegments(preset.segments.map((s) => ({ ...s, id: newSegId() })));

  // ── Copy ───────────────────────────────────────────────────────────────
  const handleCopy = () => {
    navigator.clipboard.writeText(currentOutput).then(() => {
      setCopyLabel("Copied!");
      setTimeout(() => setCopyLabel("Copy"), 2000);
    });
  };

  // ── Send / Clear ───────────────────────────────────────────────────────
  const handleSend = async () => {
    setIsSending(true);
    setSendStatus(null);
    const result = await haSetValue(haUrl, haToken, displayString);
    setSendStatus(result);
    setIsSending(false);
    if (result.ok) {
      setLiveHaString(displayString);
      setLiveHaUpdated(new Date());
    }
    setTimeout(() => setSendStatus(null), 4000);
  };

  const handleClear = async () => {
    const result = await haSetValue(haUrl, haToken, "");
    setSendStatus({
      ok: result.ok,
      message: result.ok ? "Display cleared ✓" : result.message,
    });
    if (result.ok) {
      setLiveHaString("");
      setLiveHaUpdated(new Date());
    }
    setTimeout(() => setSendStatus(null), 4000);
  };

  // ── Render ─────────────────────────────────────────────────────────────
  return (
    <div className="min-h-screen bg-[#0f1117] text-gray-100 font-sans">
      {/* Header */}
      <header className="border-b border-gray-800 px-6 py-4 flex items-center justify-between">
        <div>
          <h1 className="text-xl font-bold text-white">LED Sign Controller</h1>
          <p className="text-xs text-gray-400">
            Garage Bi-LED Display · Seeed XIAO ESP32-C3
          </p>
        </div>
        <div className="flex items-center gap-3">
          <button
            onClick={handleLoadFromHa}
            disabled={isLoading || !haToken}
            title="Load current display state from Home Assistant"
            className="px-3 py-1.5 rounded-lg bg-gray-800 hover:bg-gray-700 border border-gray-700 text-xs text-gray-300 disabled:opacity-40 disabled:cursor-not-allowed transition"
          >
            {isLoading ? "Loading…" : "Load from HA"}
          </button>
          <button
            onClick={() => setShowConfig((v) => !v)}
            className="flex items-center gap-2 px-3 py-1.5 rounded-full bg-gray-800 hover:bg-gray-700 text-sm text-gray-300 border border-gray-700 transition"
          >
            <span
              className={`w-2 h-2 rounded-full ${haToken ? "bg-green-400" : "bg-yellow-400"}`}
            />
            {haToken ? "HA Connected" : "Configure HA"}
          </button>
        </div>
      </header>

      {/* HA Config Drawer */}
      {showConfig && (
        <div className="mx-6 mt-4 p-4 rounded-xl border border-blue-800 bg-blue-950/30">
          <h3 className="text-sm font-semibold text-blue-300 mb-3">
            Home Assistant Configuration
          </h3>
          <div className="grid grid-cols-1 md:grid-cols-2 gap-3">
            <div>
              <label className="text-xs text-gray-400 block mb-1">HA URL</label>
              <input
                type="text"
                value={haUrl}
                onChange={(e) => saveHaUrl(e.target.value)}
                placeholder="http://homeassistant.local:8123"
                className="w-full bg-gray-900 border border-gray-700 rounded-lg px-3 py-2 text-sm text-gray-200 focus:outline-none focus:border-blue-500"
              />
            </div>
            <div>
              <label className="text-xs text-gray-400 block mb-1">
                Long-Lived Access Token
              </label>
              <input
                type="password"
                value={haToken}
                onChange={(e) => saveHaToken(e.target.value)}
                placeholder="Your HA long-lived access token"
                className="w-full bg-gray-900 border border-gray-700 rounded-lg px-3 py-2 text-sm text-gray-200 focus:outline-none focus:border-blue-500"
              />
            </div>
          </div>
          <p className="mt-2 text-xs text-gray-500">
            Generate a token in HA → Profile → Long-Lived Access Tokens. Entity:{" "}
            <code className="text-blue-400">{haEntity}</code>
          </p>
          <p className="mt-1 text-xs text-gray-600">
            API calls go to:{" "}
            <code className="text-gray-400">{haOrigin(haUrl)}</code>
          </p>
        </div>
      )}

      <div className="p-6 space-y-6">
        {/* Live Preview */}
        <div className="rounded-xl border border-gray-800 bg-gray-900/50 p-5">
          <div className="flex items-center justify-between mb-3">
            <div className="flex items-center gap-1 border-b border-gray-800 -mb-px">
              <button
                onClick={() => setPreviewTab("live")}
                className={`px-4 py-2 text-xs font-semibold tracking-wide border-b-2 transition -mb-px ${
                  previewTab === "live"
                    ? "border-green-500 text-green-400"
                    : "border-transparent text-gray-500 hover:text-gray-300"
                }`}
              >
                Live Display
              </button>
              <button
                onClick={() => setPreviewTab("composing")}
                className={`px-4 py-2 text-xs font-semibold tracking-wide border-b-2 transition -mb-px ${
                  previewTab === "composing"
                    ? "border-blue-500 text-blue-400"
                    : "border-transparent text-gray-500 hover:text-gray-300"
                }`}
              >
                Composing
              </button>
            </div>
            <div className="flex items-center gap-3">
              {previewTab === "live" && liveHaUpdated && (
                <span className="text-xs text-gray-500">
                  updated {liveHaUpdated.toLocaleTimeString()}
                </span>
              )}
              <span className="text-sm text-gray-400">Speed</span>
              <input
                type="range"
                min={20}
                max={300}
                value={scrollSpeed}
                onChange={(e) => setScrollSpeed(Number(e.target.value))}
                className="w-32 accent-blue-500"
              />
              <span className="text-sm text-gray-300 w-12 text-right">
                {scrollSpeed}ms
              </span>
              <button
                onClick={() => setIsPaused((v) => !v)}
                className="px-3 py-1 rounded-md bg-gray-800 hover:bg-gray-700 text-sm border border-gray-700 transition"
              >
                {isPaused ? "▶ Play" : "⏸ Pause"}
              </button>
            </div>
          </div>
          <div className="rounded-lg overflow-hidden bg-black border border-gray-800">
            {previewTab === "live" ? (
              liveHaString !== null ? (
                <LedPreview
                  text={liveHaString}
                  scrollSpeed={scrollSpeed}
                  isPaused={isPaused}
                />
              ) : (
                <div className="flex items-center justify-center h-16 text-xs text-gray-600">
                  {haToken ? "Fetching live state…" : "Configure HA to see the live display"}
                </div>
              )
            ) : (
              <LedPreview
                text={displayString}
                scrollSpeed={scrollSpeed}
                isPaused={isPaused}
              />
            )}
          </div>
        </div>

        {/* Main grid */}
        <div className="grid grid-cols-1 lg:grid-cols-[1fr_320px] gap-6">
          {/* Left column */}
          <div className="space-y-5">
            {/* Segment Builder */}
            <div className="rounded-xl border border-gray-800 bg-gray-900/50 p-5">
              <div className="flex items-center justify-between mb-4">
                <span className="text-xs font-semibold tracking-widest uppercase text-gray-300">
                  String Builder
                </span>
                <button
                  onClick={addSegment}
                  className="px-3 py-1.5 rounded-lg bg-blue-600 hover:bg-blue-500 text-sm font-medium transition"
                >
                  + Add Segment
                </button>
              </div>

              <div className="space-y-2 mb-4">
                {segments.map((seg, idx) => (
                  <div key={seg.id} className="flex items-center gap-2">
                    <span className="text-xs text-gray-600 w-4 text-right">
                      {idx + 1}
                    </span>

                    {/* Color selector */}
                    <div className="relative">
                      <select
                        value={seg.color}
                        onChange={(e) =>
                          updateSegment(seg.id, { color: e.target.value as ColorKey })
                        }
                        className="appearance-none bg-gray-800 border border-gray-700 rounded-lg pl-7 pr-6 py-1.5 text-sm text-gray-200 focus:outline-none focus:border-blue-500 cursor-pointer"
                      >
                        {(Object.keys(colorMap) as ColorKey[]).map((c) => (
                          <option key={c} value={c}>{c}</option>
                        ))}
                      </select>
                      <span
                        className="absolute left-2 top-1/2 -translate-y-1/2 w-3 h-3 rounded-full pointer-events-none"
                        style={{ background: colorMap[seg.color].dot }}
                      />
                    </div>

                    {/* Font selector */}
                    <select
                      value={seg.font}
                      onChange={(e) =>
                        updateSegment(seg.id, { font: Number(e.target.value) as FontKey })
                      }
                      className="bg-gray-800 border border-gray-700 rounded-lg px-2 py-1.5 text-sm text-gray-200 focus:outline-none focus:border-blue-500 cursor-pointer"
                    >
                      {(Object.keys(fontCodes) as unknown as FontKey[]).map((f) => (
                        <option key={f} value={f}>Font {f}</option>
                      ))}
                    </select>

                    {/* Text input */}
                    <input
                      type="text"
                      value={seg.text}
                      onChange={(e) =>
                        updateSegment(seg.id, { text: e.target.value })
                      }
                      className="flex-1 bg-gray-800 border border-gray-700 rounded-lg px-3 py-1.5 text-sm text-gray-200 focus:outline-none focus:border-blue-500"
                    />

                    <button
                      onClick={() => moveSegment(seg.id, -1)}
                      disabled={idx === 0}
                      className="w-7 h-7 flex items-center justify-center rounded bg-gray-800 hover:bg-gray-700 border border-gray-700 disabled:opacity-30 disabled:cursor-not-allowed text-xs transition"
                    >▲</button>
                    <button
                      onClick={() => moveSegment(seg.id, 1)}
                      disabled={idx === segments.length - 1}
                      className="w-7 h-7 flex items-center justify-center rounded bg-gray-800 hover:bg-gray-700 border border-gray-700 disabled:opacity-30 disabled:cursor-not-allowed text-xs transition"
                    >▼</button>
                    <button
                      onClick={() => removeSegment(seg.id)}
                      className="w-7 h-7 flex items-center justify-center rounded bg-gray-800 hover:bg-red-900/60 border border-gray-700 hover:border-red-700 text-xs transition text-gray-400 hover:text-red-400"
                    >✕</button>
                  </div>
                ))}
              </div>

              {/* Preset chips */}
              <div className="flex flex-wrap gap-2">
                {presets.map((p) => (
                  <button
                    key={p.label}
                    onClick={() => applyPreset(p)}
                    className="flex items-center gap-1.5 px-3 py-1 rounded-full bg-gray-800 hover:bg-gray-700 border border-gray-700 text-xs text-gray-300 transition"
                  >
                    <span
                      className="w-2 h-2 rounded-full inline-block"
                      style={{ background: p.dot }}
                    />
                    {p.label}
                  </button>
                ))}
              </div>
            </div>

            {/* Output panel */}
            <div className="rounded-xl border border-gray-800 bg-gray-900/50 p-5">
              <div className="flex items-center justify-between mb-3">
                <span className="text-xs font-semibold tracking-widest uppercase text-gray-300">
                  Output
                </span>
                <button
                  onClick={handleCopy}
                  className="px-3 py-1 rounded-lg bg-gray-800 hover:bg-gray-700 border border-gray-700 text-xs text-gray-300 transition"
                >
                  {copyLabel}
                </button>
              </div>
              <div className="flex border-b border-gray-800 mb-3">
                {(["plain", "yaml", "lambda"] as const).map((tab) => (
                  <button
                    key={tab}
                    onClick={() => setOutputTab(tab)}
                    className={`px-4 py-2 text-xs font-medium border-b-2 transition -mb-px ${
                      outputTab === tab
                        ? "border-blue-500 text-blue-400"
                        : "border-transparent text-gray-500 hover:text-gray-300"
                    }`}
                  >
                    {tab === "plain"
                      ? "Plain String"
                      : tab === "yaml"
                      ? "HA Service YAML"
                      : "ESPHome Lambda"}
                  </button>
                ))}
              </div>
              <pre className="bg-gray-950 rounded-lg p-4 text-xs text-amber-400 font-mono overflow-x-auto whitespace-pre-wrap break-all">
                {currentOutput}
              </pre>
            </div>
          </div>

          {/* Right column */}
          <div className="space-y-5">
            {/* Send to Display */}
            <div className="rounded-xl border border-gray-800 bg-gray-900/50 p-5">
              <span className="text-xs font-semibold tracking-widest uppercase text-gray-300 block mb-4">
                Send to Display
              </span>
              <button
                onClick={handleSend}
                disabled={isSending || !haToken}
                className="w-full py-3 rounded-xl bg-blue-600 hover:bg-blue-500 active:bg-blue-700 disabled:opacity-60 disabled:cursor-not-allowed font-semibold text-white transition text-sm shadow-lg shadow-blue-900/30"
              >
                {isSending ? "Sending…" : "Send to LED Sign"}
              </button>

              {sendStatus && (
                <div
                  className={`mt-3 px-3 py-2 rounded-lg text-xs font-medium ${
                    sendStatus.ok
                      ? "bg-green-900/40 border border-green-700 text-green-400"
                      : "bg-red-900/40 border border-red-700 text-red-400"
                  }`}
                >
                  {sendStatus.message}
                </div>
              )}

              <button
                onClick={handleClear}
                disabled={!haToken}
                className="mt-3 w-full py-2.5 rounded-xl bg-gray-800 hover:bg-gray-700 border border-gray-700 text-sm text-gray-400 hover:text-gray-200 disabled:opacity-40 disabled:cursor-not-allowed transition"
              >
                Clear Display
              </button>

              {!haToken && (
                <p className="mt-3 text-xs text-yellow-600">
                  Configure your HA URL & token (click "Configure HA") to enable
                  sending.
                </p>
              )}
            </div>

            {/* Color Codes Reference */}
            <div className="rounded-xl border border-gray-800 bg-gray-900/50 p-5">
              <span className="text-xs font-semibold tracking-widest uppercase text-gray-300 block mb-3">
                Color Codes
              </span>
              <div className="space-y-2">
                {(Object.entries(colorMap) as [ColorKey, { dot: string }][]).map(
                  ([key, val]) => (
                    <div key={key} className="flex items-center justify-between text-sm">
                      <div className="flex items-center gap-2">
                        <span
                          className="w-3 h-3 rounded-full inline-block"
                          style={{ background: val.dot }}
                        />
                        <span className="text-gray-300">
                          {colorCodesNum[key]} — {key}
                        </span>
                      </div>
                      <code className="text-gray-500 text-xs font-mono">
                        \[F;{colorCodesNum[key]}m
                      </code>
                    </div>
                  )
                )}
              </div>
            </div>

            {/* Font Codes Reference */}
            <div className="rounded-xl border border-gray-800 bg-gray-900/50 p-5">
              <span className="text-xs font-semibold tracking-widest uppercase text-gray-300 block mb-3">
                Font Codes
              </span>
              <div className="space-y-2">
                {(Object.entries(fontCodes) as [string, string][]).map(
                  ([idx, name]) => (
                    <div key={idx} className="flex items-center justify-between text-sm">
                      <span className="text-gray-300">
                        Font {idx} — {name}
                      </span>
                      <code className="text-gray-500 text-xs font-mono">
                        \[{idx};Cm
                      </code>
                    </div>
                  )
                )}
              </div>
            </div>

            {/* HA Entity Info */}
            <div className="rounded-xl border border-gray-800 bg-gray-900/50 p-4">
              <span className="text-xs font-semibold tracking-widest uppercase text-gray-300 block mb-2">
                HA Entity
              </span>
              <code className="text-blue-400 text-xs font-mono break-all">
                {haEntity}
              </code>
              <p className="text-xs text-gray-500 mt-1">
                ESPHome subscribes to this entity and updates the display on change.
              </p>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
