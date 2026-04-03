import type { Metadata } from "next";
import "./globals.css";

export const metadata: Metadata = {
  title: "minigit GUI — Version Control Dashboard",
  description:
    "A premium web-based dashboard for minigit, the C++23 custom VCS. Visualize commits, manage branches, and stage files through a beautiful graphical interface.",
};

export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html lang="en" className="h-full">
      <body className="min-h-full">{children}</body>
    </html>
  );
}
