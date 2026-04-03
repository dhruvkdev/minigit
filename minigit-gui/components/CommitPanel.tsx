"use client";

import { motion } from "framer-motion";
import { SendHorizonal, Loader2, CheckCircle2, AlertCircle } from "lucide-react";
import { useState } from "react";

interface CommitPanelProps {
  onCommit: (message: string) => Promise<{ success: boolean; error?: string }>;
  onRefresh: () => void;
}

export function CommitPanel({ onCommit, onRefresh }: CommitPanelProps) {
  const [message, setMessage] = useState("");
  const [loading, setLoading] = useState(false);
  const [result, setResult] = useState<{ ok: boolean; msg: string } | null>(null);

  const handleCommit = async () => {
    if (!message.trim()) return;
    setLoading(true);
    setResult(null);
    const res = await onCommit(message.trim());
    setLoading(false);
    if (res.success) {
      setResult({ ok: true, msg: "Committed successfully!" });
      setMessage("");
      onRefresh();
    } else {
      setResult({ ok: false, msg: res.error || "Commit failed." });
    }
    setTimeout(() => setResult(null), 4000);
  };

  return (
    <div className="glass-card p-5">
      <h2 className="text-sm font-semibold tracking-wider mb-4" style={{ color: "var(--text-secondary)" }}>
        NEW COMMIT
      </h2>

      <div className="space-y-3">
        <textarea
          className="input-field px-3 py-3 resize-none"
          rows={3}
          placeholder="Write a meaningful commit message..."
          value={message}
          onChange={e => setMessage(e.target.value)}
          onKeyDown={e => {
            if (e.key === "Enter" && (e.ctrlKey || e.metaKey)) handleCommit();
          }}
        />

        <div className="flex items-center gap-2">
          <motion.button
            className="btn-glow flex items-center gap-2 px-5 py-2 flex-1"
            onClick={handleCommit}
            disabled={loading || !message.trim()}
            whileTap={{ scale: 0.97 }}
          >
            {loading ? (
              <Loader2 size={14} className="animate-spin" />
            ) : (
              <SendHorizonal size={14} />
            )}
            {loading ? "Committing..." : "Commit"}
          </motion.button>
          <p className="text-[11px]" style={{ color: "var(--text-muted)" }}>
            Ctrl+Enter
          </p>
        </div>

        {result && (
          <motion.div
            initial={{ opacity: 0, y: 8 }}
            animate={{ opacity: 1, y: 0 }}
            exit={{ opacity: 0, y: -8 }}
            className="flex items-center gap-2 px-3 py-2 rounded-lg text-sm"
            style={{
              background: result.ok ? "rgba(16,185,129,0.08)" : "rgba(239,68,68,0.08)",
              border: `1px solid ${result.ok ? "rgba(16,185,129,0.25)" : "rgba(239,68,68,0.25)"}`,
              color: result.ok ? "var(--accent-green)" : "var(--accent-red)",
            }}
          >
            {result.ok ? <CheckCircle2 size={14} /> : <AlertCircle size={14} />}
            {result.msg}
          </motion.div>
        )}
      </div>
    </div>
  );
}
