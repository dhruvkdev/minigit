"use client";

import { motion, AnimatePresence } from "framer-motion";
import { FolderOpen, Plus, CheckSquare, GitBranch, Check, Loader2 } from "lucide-react";
import { useState } from "react";

interface StagingAreaProps {
  stagedFiles: string[];
  onStageAll: () => Promise<void>;
  onRefresh: () => void;
}

export function StagingArea({ stagedFiles, onStageAll, onRefresh }: StagingAreaProps) {
  const [staging, setStaging] = useState(false);
  const [stageSuccess, setStageSuccess] = useState(false);

  const handleStageAll = async () => {
    setStaging(true);
    setStageSuccess(false);
    await onStageAll();
    setStaging(false);
    setStageSuccess(true);
    onRefresh();
    setTimeout(() => setStageSuccess(false), 2500);
  };

  return (
    <div className="glass-card p-5">
      <div className="flex items-center justify-between mb-4">
        <h2 className="text-sm font-semibold tracking-wider" style={{ color: "var(--text-secondary)" }}>
          STAGING AREA
        </h2>
        <motion.button
          className="btn-glow flex items-center gap-2 px-3 py-2"
          onClick={handleStageAll}
          disabled={staging}
          whileHover={{ scale: 1.03 }}
          whileTap={{ scale: 0.97 }}
        >
          {staging ? (
            <Loader2 size={13} className="animate-spin" />
          ) : stageSuccess ? (
            <Check size={13} />
          ) : (
            <Plus size={13} />
          )}
          {staging ? "Staging..." : stageSuccess ? "Staged!" : "Stage All"}
        </motion.button>
      </div>

      {stagedFiles.length === 0 ? (
        <motion.div
          initial={{ opacity: 0 }}
          animate={{ opacity: 1 }}
          className="flex flex-col items-center justify-center gap-2 py-8"
        >
          <FolderOpen size={28} style={{ color: "var(--text-muted)" }} />
          <p className="text-sm" style={{ color: "var(--text-muted)" }}>
            No files staged
          </p>
          <p className="text-xs text-center px-4" style={{ color: "var(--text-muted)" }}>
            Click &quot;Stage All&quot; or use <span className="mono">mgit add &lt;file&gt;</span> in the terminal
          </p>
        </motion.div>
      ) : (
        <div className="space-y-2">
          <AnimatePresence>
            {stagedFiles.map((file, idx) => (
              <motion.div
                key={file}
                initial={{ opacity: 0, x: -12 }}
                animate={{ opacity: 1, x: 0 }}
                exit={{ opacity: 0, x: 12 }}
                transition={{ delay: idx * 0.05 }}
                className="flex items-center gap-3 px-3 py-2 rounded-lg"
                style={{
                  background: "rgba(16, 185, 129, 0.06)",
                  border: "1px solid rgba(16, 185, 129, 0.2)",
                }}
              >
                {file === "." ? (
                  <GitBranch size={14} style={{ color: "var(--accent-green)" }} />
                ) : (
                  <CheckSquare size={14} style={{ color: "var(--accent-green)" }} />
                )}
                <span
                  className="mono text-xs"
                  style={{ color: "var(--text-primary)" }}
                >
                  {file === "." ? "All files (staged with .)" : file}
                </span>
              </motion.div>
            ))}
          </AnimatePresence>
        </div>
      )}
    </div>
  );
}
